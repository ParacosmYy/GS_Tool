/**
 * @file HistogramBuilder.cpp
 * @brief 直方图构建器实现 — 等宽/等频/Sturges/FD分箱
 */

#include "utils/histogram/HistogramBuilder.h"

#include <QtMath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
HistogramBuilder::HistogramBuilder(QObject* parent)
    : QObject(parent)
    , m_strategy(BinningStrategy::Sturges)
    , m_binCount(10)
    , m_binCountSum(0.0)
{
}

/** @brief 设置分箱策略 @param strategy 策略 */
void HistogramBuilder::setBinningStrategy(BinningStrategy strategy)
{
    m_strategy = strategy;
}

/** @brief 设置bin数量 @param count bin数 */
void HistogramBuilder::setBinCount(int count)
{
    m_binCount = qMax(2, count);
}

/** @brief 构建直方图 @param data 数据 @return bin列表 */
QList<HistogramBuilder::Bin> HistogramBuilder::build(
    const QVector<double>& data)
{
    if (data.size() < 2) {
        return {};
    }

    QVector<double> sorted = data;
    std::sort(sorted.begin(), sorted.end());

    int bins = m_binCount;
    if (m_strategy == BinningStrategy::Sturges
        || m_strategy == BinningStrategy::FreedmanDiaconis) {
        bins = computeOptimalBins(sorted);
    }

    double minVal = sorted.first();
    double maxVal = sorted.last();
    double range = maxVal - minVal;
    if (qFuzzyIsNull(range)) {
        range = 1.0;
    }
    double binWidth = range / bins;

    QList<Bin> result;
    result.reserve(bins);

    if (m_strategy == BinningStrategy::EqualFrequency) {
        /* 等频分箱 */
        int perBin = data.size() / bins;
        if (perBin < 1) perBin = 1;

        for (int i = 0; i < bins; ++i) {
            Bin b;
            int start = i * perBin;
            int end = qMin((i + 1) * perBin, data.size());
            b.lowerBound = (start < sorted.size()) ? sorted[start] : maxVal;
            b.upperBound = (end < sorted.size()) ? sorted[end - 1] : maxVal;
            b.count = end - start;
            result.append(b);
        }
    } else {
        /* 等宽分箱 */
        for (int i = 0; i < bins; ++i) {
            Bin b;
            b.lowerBound = minVal + i * binWidth;
            b.upperBound = minVal + (i + 1) * binWidth;
            b.count = 0;
            result.append(b);
        }

        for (double v : sorted) {
            int idx = static_cast<int>((v - minVal) / binWidth);
            if (idx >= bins) idx = bins - 1;
            ++result[idx].count;
        }
    }

    /* 计算频率和累积频率 */
    double cumulative = 0.0;
    for (auto& b : result) {
        b.frequency = static_cast<double>(b.count) / data.size();
        cumulative += b.frequency;
        b.cumulativeFreq = cumulative;
    }

    /* 更新统计 */
    m_stats.totalValuesBinned += static_cast<quint64>(data.size());
    ++m_stats.totalHistogramsBuilt;
    m_binCountSum += static_cast<double>(result.size());
    m_stats.averageBinCount = m_binCountSum
        / static_cast<double>(m_stats.totalHistogramsBuilt);
    if (static_cast<int>(result.size()) > m_stats.peakBinCount) {
        m_stats.peakBinCount = static_cast<int>(result.size());
    }

    m_lastResult = result;
    emit histogramReady(result.size());
    return result;
}

/** @brief 自定义边界构建 @param data 数据 @param edges 边界 @return bin列表 */
QList<HistogramBuilder::Bin> HistogramBuilder::buildCustom(
    const QVector<double>& data, const QVector<double>& edges)
{
    if (edges.size() < 2 || data.isEmpty()) {
        return {};
    }

    QList<Bin> result;
    for (int i = 0; i < edges.size() - 1; ++i) {
        Bin b;
        b.lowerBound = edges[i];
        b.upperBound = edges[i + 1];
        b.count = 0;
        result.append(b);
    }

    for (double v : data) {
        for (int i = 0; i < result.size(); ++i) {
            if (v >= result[i].lowerBound && v < result[i].upperBound) {
                ++result[i].count;
                break;
            }
            if (i == result.size() - 1 && v >= result[i].upperBound) {
                ++result[i].count;
            }
        }
    }

    double cumulative = 0.0;
    for (auto& b : result) {
        b.frequency = static_cast<double>(b.count) / data.size();
        cumulative += b.frequency;
        b.cumulativeFreq = cumulative;
    }

    m_lastResult = result;
    emit histogramReady(result.size());
    return result;
}

/** @brief 获取最近结果 @return bin列表 */
QList<HistogramBuilder::Bin> HistogramBuilder::lastResult() const
{
    return m_lastResult;
}

/** @brief 重置统计 */
void HistogramBuilder::resetStatistics()
{
    m_stats = Stats{};
    m_binCountSum = 0.0;
}

/** @brief 计算最优bin数 @param sortedData 排序后数据 @return bin数 */
int HistogramBuilder::computeOptimalBins(const QVector<double>& sortedData) const
{
    int n = sortedData.size();

    if (m_strategy == BinningStrategy::Sturges) {
        return qMax(3, static_cast<int>(1 + 3.322 * std::log10(n)));
    }

    /* Freedman-Diaconis */
    int n4 = n / 4;
    double q1 = sortedData[n4];
    double q3 = sortedData[3 * n4];
    double iqr = q3 - q1;
    if (qFuzzyIsNull(iqr)) {
        return m_binCount;
    }
    double binWidth = 2.0 * iqr / qPow(n, 1.0 / 3.0);
    double range = sortedData.last() - sortedData.first();
    if (qFuzzyIsNull(binWidth)) {
        return m_binCount;
    }
    return qMax(3, static_cast<int>(qCeil(range / binWidth)));
}
