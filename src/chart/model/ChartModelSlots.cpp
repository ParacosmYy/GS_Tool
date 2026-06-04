/**
 * @file ChartModelSlots.cpp
 * @brief ChartModel 槽函数和内部辅助方法实现
 *
 * 从 ChartModel.cpp 拆分而来，包含帧数据接收回调、
 * 定时刷新回调、缓冲区重建和数据点追加。
 */

#include "chart/model/ChartModel.h"

#include <QDateTime>

// ============================================================================
// 槽函数 -- 帧数据接收
// ============================================================================

/** @brief 帧解析回调：计算各通道值、降采样、追加缓冲区
 * @param fields 解析后的字段映射
 * @param rawFrame 原始帧数据(未使用)
 */
void ChartModel::onFrameParsed(const QVariantMap& fields,
                               const QByteArray& rawFrame)
{
    Q_UNUSED(rawFrame);

    if (m_configSet.channels().isEmpty()) {
        return;
    }

    QMap<QString, double> computedValues = m_configSet.computeAll(fields);
    m_frameIndex++;

    for (auto it = computedValues.constBegin();
         it != computedValues.constEnd(); ++it) {
        const QString& channelName = it.key();
        double value = it.value();

        if (qIsNaN(value)) {
            auto bufIt = m_buffers.find(channelName);
            if (bufIt != m_buffers.end()) {
                bufIt.value().sampleCounter++;
            }
            continue;
        }

        const ChannelConfig* cfg = m_configSet.findChannel(channelName);
        int sampleDivisor = (cfg != nullptr) ? cfg->sampleDivisor : 1;
        if (sampleDivisor < 1) sampleDivisor = 1;

        appendPoint(channelName, value, sampleDivisor);
        m_totalPoints++;
    }

    if (m_refreshInterval == 0) {
        flushPendingUpdates();
    } else {
        if (!m_refreshTimer->isActive()) {
            m_refreshTimer->start(m_refreshInterval);
        }
    }
}

// ============================================================================
// 槽函数 -- 定时刷新
// ============================================================================

/** @brief 刷新定时器回调：刷出所有挂起的数据更新 */
void ChartModel::onRefreshTick()
{
    flushPendingUpdates();
}

// ============================================================================
// 内部方法
// ============================================================================

/** @brief 重建通道缓冲区(清除所有数据并为启用的通道预分配空间) */
void ChartModel::rebuildBuffers()
{
    m_buffers.clear();

    const QVector<ChannelConfig>& channels = m_configSet.channels();
    for (const ChannelConfig& cfg : channels) {
        if (cfg.enabled && !cfg.displayName.isEmpty()) {
            ChannelBuffer buf;
            buf.sampleCounter = 0;
            buf.points.reserve(m_windowSize);
            m_buffers.insert(cfg.displayName, buf);
        }
    }
}

/** @brief 向指定通道追加数据点(含降采样和滑动窗口裁剪)
 * @param displayName 通道名称
 * @param value 数据值
 * @param sampleDivisor 降采样因子
 */
void ChartModel::appendPoint(const QString& displayName, double value,
                             int sampleDivisor)
{
    auto it = m_buffers.find(displayName);
    if (it == m_buffers.end()) {
        return;
    }

    ChannelBuffer& buf = it.value();

    buf.sampleCounter++;
    if (buf.sampleCounter % sampleDivisor != 0) {
        return;
    }

    buf.points.append(QPointF(m_frameIndex, value));

    /* 滑动窗口裁剪 */
    while (buf.points.size() > m_windowSize) {
        buf.points.removeFirst();
    }

    /* 更新单通道最大数据点数统计 */
    quint64 currentSize = static_cast<quint64>(buf.points.size());
    if (currentSize > m_maxDataPointsInChannel) {
        m_maxDataPointsInChannel = currentSize;
    }

    if (!m_pendingUpdates.contains(displayName)) {
        m_pendingUpdates.append(displayName);
    }
}

/** @brief 刷出所有挂起的通道更新，发射dataUpdated信号通知ChartWidget重绘 */
void ChartModel::flushPendingUpdates()
{
    if (m_pendingUpdates.isEmpty()) {
        return;
    }

    ++m_totalDataUpdates;  ///< 统计: 数据更新信号发射次数递增
    QStringList updates = m_pendingUpdates;
    m_pendingUpdates.clear();
    emit dataUpdated(updates);
}
