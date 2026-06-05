/**
 * @file HurstExponent.cpp
 * @brief Hurst指数计算器实现 — R/S分析
 */

#include "utils/hurst/HurstExponent.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
HurstExponent::HurstExponent(QObject* parent)
    : QObject(parent), m_timeSum(0.0) {}

/** @brief 计算Hurst指数 @param data 时间序列 @param minSegment 最小段长度 @return 分析结果 */
HurstExponent::Result HurstExponent::compute(
    const QVector<double>& data, int minSegment)
{
    QElapsedTimer timer;
    timer.start();

    Result result;
    int n = data.size();
    if (n < minSegment * 2) {
        result.interpretation = tr("数据不足，需要至少%1个样本").arg(minSegment * 2);
        return result;
    }

    /* 不同窗口大小下计算R/S统计量 */
    QVector<QPair<double, double>> points;

    for (int size = minSegment; size <= n; size = size * 3 / 2 + 1) {
        int numSegments = n / size;
        if (numSegments < 1) break;

        double rsSum = 0.0;
        int validSegments = 0;

        for (int seg = 0; seg < numSegments; ++seg) {
            QVector<double> segment(size);
            for (int i = 0; i < size; ++i) {
                segment[i] = data[seg * size + i];
            }

            double rs = rsStatistic(segment);
            if (rs > 0 && std::isfinite(rs)) {
                rsSum += rs;
                validSegments++;
            }
        }

        if (validSegments > 0) {
            double avgRs = rsSum / validSegments;
            points.append({qLn(static_cast<double>(size)), qLn(avgRs)});
        }
    }

    result.rsPoints = points;

    /* 线性回归: log(R/S) = H * log(n) + c */
    if (points.size() >= 2) {
        double sumX = 0, sumY = 0, sumXY = 0, sumX2 = 0, sumY2 = 0;
        int m = points.size();
        for (int i = 0; i < m; ++i) {
            sumX += points[i].first;
            sumY += points[i].second;
            sumXY += points[i].first * points[i].second;
            sumX2 += points[i].first * points[i].first;
            sumY2 += points[i].second * points[i].second;
        }

        double denom = m * sumX2 - sumX * sumX;
        if (qAbs(denom) > 1e-12) {
            result.hurst = (m * sumXY - sumX * sumY) / denom;

            /* R^2 */
            double yMean = sumY / m;
            double ssTot = 0, ssRes = 0;
            double intercept = (sumY - result.hurst * sumX) / m;
            for (int i = 0; i < m; ++i) {
                double yPred = result.hurst * points[i].first + intercept;
                ssRes += (points[i].second - yPred) * (points[i].second - yPred);
                ssTot += (points[i].second - yMean) * (points[i].second - yMean);
            }
            result.rSquared = (ssTot > 0) ? 1.0 - ssRes / ssTot : 0.0;
        }
    }

    /* 解释 */
    if (result.hurst < 0.5) {
        result.interpretation = tr("均值回复型(H=%.3f)，具有反持续性").arg(result.hurst);
    } else if (result.hurst <= 0.55) {
        result.interpretation = tr("随机游走(H=%.3f)，布朗运动特征").arg(result.hurst);
    } else {
        result.interpretation = tr("持续型(H=%.3f)，具有长程依赖").arg(result.hurst);
    }

    m_stats.totalAnalyses++;
    m_stats.lastHurst = result.hurst;
    const auto na = m_stats.totalAnalyses;
    m_stats.avgHurst = (na == 1) ? result.hurst :
        m_stats.avgHurst * (na - 1) / na + result.hurst / na;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs =
        (m_stats.totalAnalyses > 0) ? m_timeSum / m_stats.totalAnalyses : 0.0;

    emit analysisCompleted(result.hurst, result.interpretation);
    return result;
}

/** @brief 滑动窗口Hurst分析 @param data 时间序列 @param windowSize 窗口 @param stepSize 步长 @return Hurst序列 */
QVector<double> HurstExponent::slidingWindow(
    const QVector<double>& data, int windowSize, int stepSize)
{
    QVector<double> hurstValues;
    int n = data.size();
    if (windowSize < 8 || n < windowSize) return hurstValues;

    if (stepSize <= 0) stepSize = windowSize / 4;

    for (int start = 0; start + windowSize <= n; start += stepSize) {
        QVector<double> window(windowSize);
        for (int i = 0; i < windowSize; ++i) {
            window[i] = data[start + i];
        }
        Result r = compute(window);
        hurstValues.append(r.hurst);
    }
    return hurstValues;
}

/** @brief R/S统计量 @param data 数据段 @return R/S值 */
double HurstExponent::rsStatistic(const QVector<double>& data)
{
    int n = data.size();
    if (n < 2) return 0.0;

    /* 计算累积离差 */
    double mean = 0.0;
    for (int i = 0; i < n; ++i) mean += data[i];
    mean /= n;

    /* 累积和序列 */
    QVector<double> cumSum(n, 0.0);
    cumSum[0] = data[0] - mean;
    for (int i = 1; i < n; ++i) {
        cumSum[i] = cumSum[i - 1] + (data[i] - mean);
    }

    /* 极差R */
    double maxCum = *std::max_element(cumSum.begin(), cumSum.end());
    double minCum = *std::min_element(cumSum.begin(), cumSum.end());
    double R = maxCum - minCum;

    /* 标准差S */
    double var = 0.0;
    for (int i = 0; i < n; ++i) {
        double d = data[i] - mean;
        var += d * d;
    }
    double S = qSqrt(var / n);

    return (S > 0) ? R / S : 0.0;
}

/** @brief 重置统计 */
void HurstExponent::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
