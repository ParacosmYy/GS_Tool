/**
 * @file DataSegmentAnalyzer.cpp
 * @brief 数据分段分析引擎实现 — 自动分段+段特征统计
 */

#include "utils/segment/DataSegmentAnalyzer.h"

#include <QtMath>

/** @brief 构造函数 @param parent 父对象 */
DataSegmentAnalyzer::DataSegmentAnalyzer(QObject* parent)
    : QObject(parent)
    , m_policy(SegmentPolicy::Threshold)
    , m_threshold(0.5)
    , m_fixedLength(100)
    , m_minSegmentLength(5)
{
}

/** @brief 设置分段策略 @param policy 策略 */
void DataSegmentAnalyzer::setPolicy(SegmentPolicy policy)
{
    m_policy = policy;
}

/** @brief 设置分段阈值 @param threshold 阈值 */
void DataSegmentAnalyzer::setThreshold(double threshold)
{
    m_threshold = threshold;
}

/** @brief 设置固定段长度 @param length 点数 */
void DataSegmentAnalyzer::setFixedLength(int length)
{
    m_fixedLength = qMax(2, length);
}

/** @brief 设置最小段长度 @param length 最小点数 */
void DataSegmentAnalyzer::setMinSegmentLength(int length)
{
    m_minSegmentLength = qMax(1, length);
}

/** @brief 分析数据流 @param data 数据 @return 段列表 */
QList<DataSegmentAnalyzer::Segment> DataSegmentAnalyzer::analyze(
    const QVector<double>& data)
{
    if (data.size() < 2) {
        return {};
    }

    QList<Segment> segments;
    switch (m_policy) {
    case SegmentPolicy::Threshold:
        segments = analyzeThreshold(data);
        break;
    case SegmentPolicy::Variance:
        segments = analyzeVariance(data);
        break;
    case SegmentPolicy::Trend:
        segments = analyzeTrend(data);
        break;
    case SegmentPolicy::Fixed:
        segments = analyzeFixed(data);
        break;
    }

    /* 更新统计 */
    m_stats.totalPointsAnalyzed += static_cast<quint64>(data.size());
    m_stats.totalSegmentsFound += static_cast<quint64>(segments.size());

    if (!segments.isEmpty()) {
        double totalLen = 0.0;
        for (const auto& seg : segments) {
            int len = seg.endIndex - seg.startIndex + 1;
            totalLen += len;
            if (len > m_stats.peakSegmentLength) {
                m_stats.peakSegmentLength = len;
            }
            if (m_stats.shortestSegmentLength == 0
                || len < m_stats.shortestSegmentLength) {
                m_stats.shortestSegmentLength = len;
            }
        }
        m_stats.averageSegmentLength = totalLen
            / static_cast<double>(segments.size());
    }

    m_lastSegments = segments;
    emit segmentsDetected(segments);
    return segments;
}

/** @brief 获取最近分析结果 @return 段列表 */
QList<DataSegmentAnalyzer::Segment> DataSegmentAnalyzer::lastSegments() const
{
    return m_lastSegments;
}

/** @brief 重置统计 */
void DataSegmentAnalyzer::resetStatistics()
{
    m_stats = Stats{};
}

/** @brief 阈值分段 @param data 数据 @return 段列表 */
QList<DataSegmentAnalyzer::Segment> DataSegmentAnalyzer::analyzeThreshold(
    const QVector<double>& data)
{
    QList<Segment> segments;
    int segStart = 0;

    for (int i = 1; i < data.size(); ++i) {
        double diff = qAbs(data[i] - data[i - 1]);
        if (diff > m_threshold) {
            if ((i - 1) - segStart + 1 >= m_minSegmentLength) {
                segments.append(computeSegment(data, segStart, i - 1));
            }
            segStart = i;
        }
    }

    /* 最后一段 */
    if (data.size() - 1 - segStart + 1 >= m_minSegmentLength) {
        segments.append(computeSegment(data, segStart, data.size() - 1));
    }

    return segments;
}

/** @brief 方差分段 @param data 数据 @return 段列表 */
QList<DataSegmentAnalyzer::Segment> DataSegmentAnalyzer::analyzeVariance(
    const QVector<double>& data)
{
    QList<Segment> segments;
    int windowSize = qMax(5, data.size() / 20);
    int segStart = 0;
    double prevVar = 0.0;

    /* 初始窗口方差 */
    int initEnd = qMin(windowSize, data.size());
    for (int i = 0; i < initEnd; ++i) {
        prevVar += data[i] * data[i];
    }
    double initMean = 0.0;
    for (int i = 0; i < initEnd; ++i) {
        initMean += data[i];
    }
    initMean /= initEnd;
    prevVar = 0.0;
    for (int i = 0; i < initEnd; ++i) {
        double d = data[i] - initMean;
        prevVar += d * d;
    }
    prevVar /= initEnd;

    for (int i = windowSize; i < data.size(); i += windowSize / 2) {
        int end = qMin(i + windowSize, data.size());
        double mean = 0.0;
        for (int j = i; j < end; ++j) {
            mean += data[j];
        }
        mean /= (end - i);
        double var = 0.0;
        for (int j = i; j < end; ++j) {
            double d = data[j] - mean;
            var += d * d;
        }
        var /= (end - i);

        /* 方差突变检测 */
        if (prevVar > 0 && qAbs(var - prevVar) / prevVar > m_threshold * 10.0) {
            if ((i - 1) - segStart + 1 >= m_minSegmentLength) {
                segments.append(computeSegment(data, segStart, i - 1));
            }
            segStart = i;
        }
        prevVar = var;
    }

    if (data.size() - 1 - segStart + 1 >= m_minSegmentLength) {
        segments.append(computeSegment(data, segStart, data.size() - 1));
    }

    return segments;
}

/** @brief 趋势分段 @param data 数据 @return 段列表 */
QList<DataSegmentAnalyzer::Segment> DataSegmentAnalyzer::analyzeTrend(
    const QVector<double>& data)
{
    if (data.size() < 3) {
        return {computeSegment(data, 0, data.size() - 1)};
    }

    QList<Segment> segments;
    int segStart = 0;
    int prevDir = 0; /* -1=下降, 0=平坦, 1=上升 */

    double diff0 = data[1] - data[0];
    prevDir = (diff0 > m_threshold) ? 1 : ((diff0 < -m_threshold) ? -1 : 0);

    for (int i = 2; i < data.size(); ++i) {
        double diff = data[i] - data[i - 1];
        int dir = (diff > m_threshold) ? 1 : ((diff < -m_threshold) ? -1 : 0);

        if (dir != 0 && dir != prevDir && prevDir != 0) {
            if ((i - 1) - segStart + 1 >= m_minSegmentLength) {
                segments.append(computeSegment(data, segStart, i - 1));
            }
            segStart = i - 1;
        }
        if (dir != 0) {
            prevDir = dir;
        }
    }

    if (data.size() - 1 - segStart + 1 >= m_minSegmentLength) {
        segments.append(computeSegment(data, segStart, data.size() - 1));
    }

    return segments;
}

/** @brief 固定长度分段 @param data 数据 @return 段列表 */
QList<DataSegmentAnalyzer::Segment> DataSegmentAnalyzer::analyzeFixed(
    const QVector<double>& data)
{
    QList<Segment> segments;
    for (int i = 0; i < data.size(); i += m_fixedLength) {
        int end = qMin(i + m_fixedLength - 1, data.size() - 1);
        if (end - i + 1 >= m_minSegmentLength) {
            segments.append(computeSegment(data, i, end));
        }
    }
    return segments;
}

/** @brief 计算单个段的统计特征 @param data 数据 @param start 起始 @param end 结束 @return 段 */
DataSegmentAnalyzer::Segment DataSegmentAnalyzer::computeSegment(
    const QVector<double>& data, int start, int end) const
{
    Segment seg;
    seg.startIndex = start;
    seg.endIndex = end;

    int n = end - start + 1;
    double sum = 0.0;
    double minVal = data[start];
    double maxVal = data[start];

    for (int i = start; i <= end; ++i) {
        sum += data[i];
        if (data[i] < minVal) minVal = data[i];
        if (data[i] > maxVal) maxVal = data[i];
    }

    seg.mean = sum / n;
    seg.minValue = minVal;
    seg.maxValue = maxVal;

    /* 方差 */
    double sqSum = 0.0;
    for (int i = start; i <= end; ++i) {
        double d = data[i] - seg.mean;
        sqSum += d * d;
    }
    seg.variance = (n > 1) ? sqSum / (n - 1) : 0.0;

    /* 线性趋势斜率(最小二乘) */
    if (n > 1) {
        double sumX = 0.0, sumY = 0.0, sumXY = 0.0, sumX2 = 0.0;
        for (int i = start; i <= end; ++i) {
            double x = static_cast<double>(i - start);
            sumX += x;
            sumY += data[i];
            sumXY += x * data[i];
            sumX2 += x * x;
        }
        double denom = n * sumX2 - sumX * sumX;
        seg.slope = (qFuzzyIsNull(denom)) ? 0.0 : (n * sumXY - sumX * sumY) / denom;
    }

    return seg;
}
