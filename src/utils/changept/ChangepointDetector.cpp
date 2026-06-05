/**
 * @file ChangepointDetector.cpp
 * @brief 变点检测器实现
 */

#include "utils/changept/ChangepointDetector.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

ChangepointDetector::ChangepointDetector(QObject* parent)
    : QObject(parent), m_method(Method::CUSUM),
      m_penalty(10.0), m_minSegmentLength(5),
      m_windowSize(20), m_confidenceThreshold(0.5),
      m_timeSum(0.0) {}

void ChangepointDetector::setMethod(Method m) { m_method = m; }
void ChangepointDetector::setPenalty(double p) { m_penalty = p; }
void ChangepointDetector::setMinSegmentLength(int len) { m_minSegmentLength = qMax(1, len); }
void ChangepointDetector::setWindowSize(int s) { m_windowSize = qMax(2, s); }
void ChangepointDetector::setConfidenceThreshold(double t) { m_confidenceThreshold = qBound(0.0, t, 1.0); }

QList<ChangepointDetector::Changepoint> ChangepointDetector::detect(const QVector<double>& data)
{
    QElapsedTimer timer;
    timer.start();

    QList<Changepoint> result;
    if (data.size() < 2 * m_minSegmentLength) return result;

    switch (m_method) {
    case Method::CUSUM:            result = cusum(data); break;
    case Method::PELT:             result = pelt(data); break;
    case Method::SlidingWindow:    result = slidingWindow(data); break;
    case Method::BinarySegmentation: result = binarySegmentation(data); break;
    }

    double elapsed = timer.elapsed();
    ++m_stats.totalDetections;
    m_stats.totalChangepointsFound += result.size();
    m_stats.totalPointsProcessed += data.size();
    m_timeSum += elapsed;
    m_stats.averageProcessingTimeMs = m_timeSum / m_stats.totalDetections;

    emit detectionComplete(result.size());
    return result;
}

QList<ChangepointDetector::Changepoint> ChangepointDetector::cusum(const QVector<double>& data)
{
    QList<Changepoint> result;
    int n = data.size();
    if (n < 2) return result;

    /* 计算基准均值 */
    double refSum = 0.0;
    int refLen = qMin(m_minSegmentLength, n);
    for (int i = 0; i < refLen; ++i) refSum += data[i];
    double refMean = refSum / refLen;

    double cusumPos = 0.0;
    double cusumNeg = 0.0;
    double threshold = m_penalty;

    for (int i = refLen; i < n; ++i) {
        double deviation = data[i] - refMean;
        cusumPos = qMax(0.0, cusumPos + deviation);
        cusumNeg = qMin(0.0, cusumNeg + deviation);

        if (cusumPos > threshold || qAbs(cusumNeg) > threshold) {
            double confidence = qMax(cusumPos, qAbs(cusumNeg)) / (threshold * 2.0);
            confidence = qMin(1.0, confidence);

            if (confidence >= m_confidenceThreshold) {
                double mag = 0.0;
                int prevStart = qMax(0, i - m_minSegmentLength);
                double prevMean = 0.0;
                for (int j = prevStart; j < i; ++j) prevMean += data[j];
                prevMean /= (i - prevStart);
                mag = qAbs(data[i] - prevMean);

                Changepoint cp{i, confidence, mag};
                result.append(cp);
                emit changepointFound(cp);
            }

            /* 重置CUSUM并更新参考均值 */
            cusumPos = 0.0;
            cusumNeg = 0.0;
            int updateLen = qMin(m_minSegmentLength, n - i - 1);
            if (updateLen > 0) {
                double newSum = 0.0;
                for (int j = i; j < i + updateLen; ++j) newSum += data[j];
                refMean = newSum / updateLen;
            }
        }
    }
    return result;
}

QList<ChangepointDetector::Changepoint> ChangepointDetector::pelt(const QVector<double>& data)
{
    QList<Changepoint> result;
    int n = data.size();
    if (n < 2 * m_minSegmentLength) return result;

    /* PELT 动态规划 */
    QVector<double> cost(n + 1, std::numeric_limits<double>::max());
    QVector<int> lastChangepoint(n + 1, 0);
    cost[0] = 0.0;

    /* 预计算累积和用于快速计算段代价 */
    QVector<double> cumSum(n + 1, 0.0);
    QVector<double> cumSqSum(n + 1, 0.0);
    for (int i = 0; i < n; ++i) {
        cumSum[i + 1] = cumSum[i] + data[i];
        cumSqSum[i + 1] = cumSqSum[i] + data[i] * data[i];
    }

    for (int j = 1; j <= n; ++j) {
        int minStart = qMax(0, j - n); // 无实际限制
        for (int t = 0; t < j; ++t) {
            int segLen = j - t;
            if (segLen < m_minSegmentLength && t > 0) continue;

            double segCost = segmentCost(data, t, j - 1);
            double totalCost = cost[t] + segCost + m_penalty;
            if (totalCost < cost[j]) {
                cost[j] = totalCost;
                lastChangepoint[j] = t;
            }
        }
    }

    /* 回溯变点 */
    QList<int> changepoints;
    int pos = n;
    while (pos > 0) {
        int prev = lastChangepoint[pos];
        if (prev > 0 && prev < pos) {
            changepoints.prepend(prev);
        }
        pos = prev;
    }

    for (int cp : changepoints) {
        double confidence = 0.8;
        double mag = 0.0;
        if (cp > 0 && cp < n) {
            double beforeMean = cumSum[cp] / cp;
            double afterMean = (cumSum[n] - cumSum[cp]) / (n - cp);
            mag = qAbs(afterMean - beforeMean);
        }
        Changepoint c{cp, confidence, mag};
        result.append(c);
        emit changepointFound(c);
    }
    return result;
}

QList<ChangepointDetector::Changepoint> ChangepointDetector::slidingWindow(const QVector<double>& data)
{
    QList<Changepoint> result;
    int n = data.size();
    int halfW = m_windowSize / 2;

    for (int i = halfW; i < n - halfW; i += qMax(1, halfW / 2)) {
        double leftSum = 0.0, rightSum = 0.0;
        for (int j = i - halfW; j < i; ++j) leftSum += data[j];
        for (int j = i; j < i + halfW; ++j) rightSum += data[j];
        double leftMean = leftSum / halfW;
        double rightMean = rightSum / halfW;

        double leftSqSum = 0.0, rightSqSum = 0.0;
        for (int j = i - halfW; j < i; ++j) { double d = data[j] - leftMean; leftSqSum += d * d; }
        for (int j = i; j < i + halfW; ++j) { double d = data[j] - rightMean; rightSqSum += d * d; }

        double pooledVar = (leftSqSum + rightSqSum) / (2 * halfW - 2);
        if (pooledVar < 1e-10) continue;

        double tStat = qAbs(leftMean - rightMean) / qSqrt(pooledVar * 2.0 / halfW);
        double confidence = qMin(1.0, tStat / 5.0);

        if (confidence >= m_confidenceThreshold && tStat > m_penalty) {
            /* 避免重复检测相邻变点 */
            if (result.isEmpty() || qAbs(i - result.last().index) > m_minSegmentLength) {
                Changepoint cp{i, confidence, qAbs(rightMean - leftMean)};
                result.append(cp);
                emit changepointFound(cp);
            }
        }
    }
    return result;
}

QList<ChangepointDetector::Changepoint> ChangepointDetector::binarySegmentation(const QVector<double>& data)
{
    QList<Changepoint> result;
    QList<QPair<int, int>> queue;
    queue.append({0, data.size() - 1});

    while (!queue.isEmpty()) {
        auto [start, end] = queue.takeFirst();
        int segLen = end - start + 1;
        if (segLen < 2 * m_minSegmentLength) continue;

        double fullCost = segmentCost(data, start, end);
        double bestGain = 0.0;
        int bestPos = -1;

        for (int t = start + m_minSegmentLength; t <= end - m_minSegmentLength; ++t) {
            double splitCost = segmentCost(data, start, t - 1) + segmentCost(data, t, end);
            double gain = fullCost - splitCost;
            if (gain > bestGain) { bestGain = gain; bestPos = t; }
        }

        if (bestGain > m_penalty && bestPos >= 0) {
            double confidence = qMin(1.0, bestGain / (m_penalty * 3.0));
            if (confidence >= m_confidenceThreshold) {
                double mag = 0.0;
                if (bestPos > start && bestPos <= end) {
                    double leftSum = 0.0, rightSum = 0.0;
                    for (int i = start; i < bestPos; ++i) leftSum += data[i];
                    for (int i = bestPos; i <= end; ++i) rightSum += data[i];
                    mag = qAbs(rightSum / (end - bestPos + 1) - leftSum / (bestPos - start));
                }

                Changepoint cp{bestPos, confidence, mag};
                result.append(cp);
                emit changepointFound(cp);

                queue.append({start, bestPos - 1});
                queue.append({bestPos, end});
            }
        }
    }

    std::sort(result.begin(), result.end(),
              [](const Changepoint& a, const Changepoint& b) { return a.index < b.index; });
    return result;
}

double ChangepointDetector::segmentCost(const QVector<double>& data, int start, int end) const
{
    if (end < start) return 0.0;
    double sum = 0.0;
    for (int i = start; i <= end; ++i) sum += data[i];
    double mean = sum / (end - start + 1);

    double sqSum = 0.0;
    for (int i = start; i <= end; ++i) {
        double d = data[i] - mean;
        sqSum += d * d;
    }
    return sqSum;
}

void ChangepointDetector::resetStatistics() { m_stats = Stats{}; m_timeSum = 0.0; }
