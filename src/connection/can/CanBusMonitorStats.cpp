/**
 * @file CanBusMonitorStats.cpp
 * @brief CAN总线监控统计方法实现
 *
 * 从 CanBusMonitorFrame.cpp 拆分而来，包含统计摘要、
 * 帧率计算、统计面板更新和统计重置方法。
 */

#include "connection/can/CanBusMonitor.h"
#include <QTime>

/** @brief 获取统计摘要文本 @return 格式化的CAN帧统计信息 */
QString CanBusMonitor::statisticsSummary() const
{
    QString summary;
    summary += tr("总帧数: %1\n").arg(m_totalFramesMonitored);
    summary += tr("标准帧: %1 | 扩展帧: %2\n")
        .arg(m_totalStandardFrames).arg(m_totalExtendedFrames);
    summary += tr("CAN-FD帧: %1 | RTR帧: %2\n")
        .arg(m_totalFdFrames).arg(m_totalRtrFrames);
    summary += tr("总字节: %1 | 帧率: %2 fps\n")
        .arg(m_totalBytesReceived).arg(frameRate(), 0, 'f', 1);
    summary += tr("不同帧ID: %1\n").arg(m_idFrequency.size());

    if (!m_idFrequency.isEmpty()) {
        quint32 topId = 0;
        int topCount = 0;
        for (auto it = m_idFrequency.constBegin();
             it != m_idFrequency.constEnd(); ++it) {
            if (it.value() > topCount) {
                topId = it.key();
                topCount = it.value();
            }
        }
        summary += tr("最频繁帧ID: 0x%1 (%2次)")
                      .arg(topId, 0, 16).arg(topCount);
    }

    return summary.trimmed();
}

/** @brief 计算帧率(fps) @return 每秒帧数 */
double CanBusMonitor::frameRate() const
{
    const qint64 elapsed = m_rateTimer.elapsed();
    if (elapsed <= 0) return 0.0;
    return (static_cast<double>(m_rateFrameCount) * 1000.0)
           / static_cast<double>(elapsed);
}

/** @brief 更新统计面板标签文本 */
void CanBusMonitor::updateStatsDisplay()
{
    m_statsLabel->setText(
        tr("STD: %1 | EXT: %2 | FD: %3 | RTR: %4 | ERR: %5 | Bytes: %6 | %7 fps")
            .arg(m_totalStandardFrames)
            .arg(m_totalExtendedFrames)
            .arg(m_totalFdFrames)
            .arg(m_totalRtrFrames)
            .arg(m_totalErrors)
            .arg(m_totalBytesReceived)
            .arg(frameRate(), 0, 'f', 1));
}

/** @brief 重置所有统计计数器 */
void CanBusMonitor::resetStatistics()
{
    m_totalFramesMonitored = 0;
    m_totalErrors = 0;
    m_totalStandardFrames = 0;
    m_totalExtendedFrames = 0;
    m_totalFdFrames = 0;
    m_totalRtrFrames = 0;
    m_totalBytesReceived = 0;
    m_rateFrameCount = 0;
    m_rateTimer.restart();
    updateStatsDisplay();
}
