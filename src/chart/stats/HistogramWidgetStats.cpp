/**
 * @file HistogramWidgetStats.cpp
 * @brief 直方图统计计算、导出和计数器接口实现
 *
 * 从HistogramWidget.cpp拆分而来，包含:
 *   - computeHistogram(): 分桶计算
 *   - computeStats(): 统计摘要(均值/标准差/最小/最大/计数)
 *   - exportToCsv(): CSV导出
 *   - 所有统计计数器getter和重置方法
 */

#include "chart/stats/HistogramWidget.h"
#include "chart/model/ChartModel.h"

#include <QFile>
#include <QTextStream>
#include <algorithm>
#include <cmath>

/**
 * @brief 计算直方图分桶数据
 * @param data 输入数据点集(取QPointF的y分量作为值)
 * @param bins 分桶数量
 * @return 每个桶的中心值和计数的有序列表
 */
QVector<QPair<double, int>> HistogramWidget::computeHistogram(
    const QVector<QPointF>& data, int bins)
{
    QVector<QPair<double, int>> result;
    if (data.isEmpty() || bins <= 0) {
        return result;
    }

    double minVal = data.first().y();
    double maxVal = data.first().y();
    for (const auto& pt : data) {
        minVal = std::min(minVal, pt.y());
        maxVal = std::max(maxVal, pt.y());
    }

    double range = maxVal - minVal;
    if (range < 1e-12) {
        result.append(qMakePair(minVal, data.size()));
        return result;
    }

    double binWidth = range / bins;
    result.resize(bins);
    for (int i = 0; i < bins; ++i) {
        result[i] = qMakePair(minVal + (i + 0.5) * binWidth, 0);
    }

    for (const auto& pt : data) {
        int idx = static_cast<int>((pt.y() - minVal) / binWidth);
        idx = qBound(0, idx, bins - 1);
        result[idx].second++;
    }
    return result;
}

/**
 * @brief 计算数据统计摘要(均值/标准差/最小/最大/计数)
 * @param data 输入数据点集(取QPointF的y分量作为值)
 * @return 包含count/min/max/mean/stddev的Stats结构体
 */
HistogramWidget::Stats HistogramWidget::computeStats(
    const QVector<QPointF>& data)
{
    Stats s{};
    if (data.isEmpty()) {
        return s;
    }

    s.count = data.size();
    s.min = data.first().y();
    s.max = data.first().y();
    double sum = 0.0;
    for (const auto& pt : data) {
        double val = pt.y();
        sum += val;
        s.min = std::min(s.min, val);
        s.max = std::max(s.max, val);
    }
    s.mean = sum / s.count;

    double sqSum = 0.0;
    for (const auto& pt : data) {
        double diff = pt.y() - s.mean;
        sqSum += diff * diff;
    }
    s.stddev = std::sqrt(sqSum / s.count);
    return s;
}

/** @brief 导出当前直方图数据到CSV文件 @param filePath 目标文件路径 @return true=导出成功 */
bool HistogramWidget::exportToCsv(const QString& filePath)
{
    ++m_totalExports;
    if (!m_model || !m_barSet || m_barSet->count() == 0) {
        return false;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return false;
    }

    QTextStream stream(&file);
    stream << tr("桶中心值") << "," << tr("计数") << "\n";

    QString channel = m_channelCombo->currentText();
    QVector<QPointF> data = m_model->channelData(channel);
    int bins = m_binsSpin->value();
    auto histData = computeHistogram(data, bins);

    for (const auto& [center, count] : histData) {
        stream << QStringLiteral("%1,%2\n")
                      .arg(center, 0, 'f', 6)
                      .arg(count);
    }

    file.close();
    return true;
}

/** @brief 获取累计刷新更新次数 @return 更新次数 */
quint64 HistogramWidget::totalUpdates() const
{
    return m_totalUpdates;
}

/** @brief 获取累计分桶数变更次数 @return 变更次数 */
quint64 HistogramWidget::totalBinChanges() const
{
    return m_totalBinChanges;
}

/** @brief 获取累计分桶计算次数 @return 计算次数 */
quint64 HistogramWidget::totalBinsComputed() const
{
    return m_totalBinsComputed;
}

/** @brief 获取峰值所在桶索引 @return 桶索引，无数据返回-1 */
int HistogramWidget::peakBinIndex() const
{
    return m_peakBinIndex;
}

/** @brief 获取最大桶计数值 @return 最大计数 */
int HistogramWidget::maxBinCount() const
{
    return m_maxBinCount;
}

/** @brief 重置所有直方图统计计数器 */
void HistogramWidget::resetHistogramStatistics()
{
    m_totalUpdates = 0;
    m_totalBinChanges = 0;
    m_totalBinsComputed = 0;
    m_peakBinIndex = -1;
    m_maxBinCount = 0;
    m_totalBinRecalculations = 0;
    m_totalDistributionUpdates = 0;
    m_totalAutoRanges = 0;
    m_totalExports = 0;
    m_totalChannelSwitches = 0;
    m_totalAutoRefreshToggles = 0;
    m_totalThemeChanges = 0;
}

// ── 内联统计 Getter(从.h移出) ──
quint64 HistogramWidget::totalBinRecalculations() const { return m_totalBinRecalculations; }
quint64 HistogramWidget::totalDistributionUpdates() const { return m_totalDistributionUpdates; }
quint64 HistogramWidget::totalAutoRanges() const { return m_totalAutoRanges; }
quint64 HistogramWidget::totalExports() const { return m_totalExports; }
quint64 HistogramWidget::totalChannelSwitches() const { return m_totalChannelSwitches; }
quint64 HistogramWidget::totalAutoRefreshToggles() const { return m_totalAutoRefreshToggles; }
quint64 HistogramWidget::totalThemeChanges() const { return m_totalThemeChanges; }
