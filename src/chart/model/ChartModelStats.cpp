/**
 * @file ChartModelStats.cpp
 * @brief ChartModel 数据查询和统计接口
 *
 * 从 ChartModel.cpp 拆分而来，包含所有数据查询方法、
 * Y值范围计算、通道列表获取和统计计数器。
 */

#include "chart/model/ChartModel.h"
#include <QtMath>
#include <algorithm>

// ============================================================================
// 数据查询接口
// ============================================================================

/** @brief 获取指定通道的完整数据点序列 @param displayName 通道显示名 */
QVector<QPointF> ChartModel::channelData(const QString& displayName) const
{
    auto it = m_buffers.constFind(displayName);
    if (it == m_buffers.constEnd()) return QVector<QPointF>();
    return it.value().points;
}

/** @brief 获取所有启用通道的数据 @return 通道名→数据点序列的映射 */
QMap<QString, QVector<QPointF>> ChartModel::allChannelData() const
{
    QMap<QString, QVector<QPointF>> result;
    for (auto it = m_buffers.constBegin(); it != m_buffers.constEnd(); ++it) {
        result[it.key()] = it.value().points;
    }
    return result;
}

/** @brief 获取指定通道的Y值范围 @param displayName 通道显示名 @return <最小值,最大值>，无数据返回<0,0> */
QPair<double, double> ChartModel::channelYRange(const QString& displayName) const
{
    auto it = m_buffers.constFind(displayName);
    if (it == m_buffers.constEnd() || it.value().points.isEmpty()) {
        return qMakePair(0.0, 0.0);
    }

    double yMin = std::numeric_limits<double>::max();
    double yMax = std::numeric_limits<double>::lowest();
    for (const QPointF& pt : it.value().points) {
        if (!qIsNaN(pt.y()) && !qIsInf(pt.y())) {
            if (pt.y() < yMin) yMin = pt.y();
            if (pt.y() > yMax) yMax = pt.y();
        }
    }

    if (yMin > yMax) return qMakePair(0.0, 0.0);
    return qMakePair(yMin, yMax);
}

/** @brief 获取所有通道的全局Y值范围 @return <全局最小值,全局最大值>，无数据返回<0,0> */
QPair<double, double> ChartModel::globalYRange() const
{
    double globalMin = std::numeric_limits<double>::max();
    double globalMax = std::numeric_limits<double>::lowest();
    bool hasValidData = false;

    for (auto it = m_buffers.constBegin(); it != m_buffers.constEnd(); ++it) {
        for (const QPointF& pt : it.value().points) {
            if (!qIsNaN(pt.y()) && !qIsInf(pt.y())) {
                if (pt.y() < globalMin) globalMin = pt.y();
                if (pt.y() > globalMax) globalMax = pt.y();
                hasValidData = true;
            }
        }
    }

    if (!hasValidData) return qMakePair(0.0, 0.0);
    return qMakePair(globalMin, globalMax);
}

/** @brief 获取所有通道的显示名称列表 @return 通道名称字符串列表 */
QStringList ChartModel::channelNames() const { return m_buffers.keys(); }

/** @brief 获取当前X轴范围(基于帧索引和窗口大小) @return <起始值,结束值> */
QPair<double, double> ChartModel::xRange() const
{
    if (m_frameIndex <= 0) return qMakePair(0.0, 10.0);
    if (m_frameIndex >= m_windowSize) {
        return qMakePair(static_cast<double>(m_frameIndex - m_windowSize),
                         static_cast<double>(m_frameIndex));
    }
    return qMakePair(0.0, static_cast<double>(m_frameIndex) + 10.0);
}

/** @brief 获取累计接收的数据点总数 @return 数据点计数 */
qint64 ChartModel::totalPointsReceived() const { return m_totalPoints; }

/** @brief 获取当前帧索引(已解析的帧序号) @return 帧索引 */
qint64 ChartModel::currentFrameIndex() const { return m_frameIndex; }

// ============================================================================
// 统计接口
// ============================================================================

/** @brief 获取累计处理的数据点总数(含历史) @return 数据点计数 */
quint64 ChartModel::totalDataPoints() const { return m_totalDataPoints; }

/** @brief 获取累计创建的通道数 @return 通道创建计数 */
quint64 ChartModel::channelsCreated() const { return m_channelsCreated; }

/** @brief 获取累计移除的通道数 @return 通道移除计数 */
quint64 ChartModel::channelsRemoved() const { return m_channelsRemoved; }

/** @brief 获取当前活跃通道数量 @return 活跃通道计数 */
quint64 ChartModel::totalChannelsActive() const { return static_cast<quint64>(m_buffers.size()); }

/** @brief 获取所有通道中数据点数量的最大值 @return 最大数据点数 */
quint64 ChartModel::maxDataPointsInChannel() const { return m_maxDataPointsInChannel; }

/** @brief 获取峰值数据速率(点/秒) @return 峰值速率 */
double ChartModel::peakDataRate() const { return m_peakDataRate; }

/** @brief 重置所有图表统计计数器(不影响通道数据和配置) */
void ChartModel::resetChartStatistics()
{
    m_totalDataPoints = 0;
    m_channelsCreated = 0;
    m_channelsRemoved = 0;
    m_totalChannelsActive = 0;
    m_maxDataPointsInChannel = 0;
    m_peakDataRate = 0.0;
    m_dataRateTimestamp = 0;
    m_dataRatePointCount = 0;
}

/** @brief 重置统计计数器的别名，委托给resetChartStatistics() */
void ChartModel::resetStats() { resetChartStatistics(); }
