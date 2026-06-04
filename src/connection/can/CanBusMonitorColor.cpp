/**
 * @file CanBusMonitorColor.cpp
 * @brief CAN总线监控面板 — 帧颜色编码与清空操作
 *
 * 本文件拆分自 CanBusMonitor.cpp，包含:
 *   - clearFrames: 清空帧记录
 *   - rowBrush: 根据帧类型获取背景色
 *
 * 帧添加(addFrame)见 CanBusMonitorFrame.cpp。
 * 统计方法见 CanBusMonitorStats.cpp。
 */

#include "connection/can/CanBusMonitor.h"
#include "core/theme/ThemeManager.h"
#include <QBrush>
#include <QColor>

/** @brief 清空所有帧记录并重置帧计数器和频率统计 */
void CanBusMonitor::clearFrames()
{
    m_frameTable->setRowCount(0);
    m_signalTable->setRowCount(0);
    m_frameCount = 0;
    m_rateFrameCount = 0;
    m_idFrequency.clear();
    m_rateTimer.restart();
    m_countLabel->setText(tr("帧数: 0"));
    updateStatsDisplay();
}

/** @brief 根据帧类型获取行背景色(FD浅绿/RTR黄色/扩展帧浅蓝/错误帧红色/标准帧白色) @param frame CAN帧 @return 背景QBrush */
QBrush CanBusMonitor::rowBrush(const CanFrame& frame) const
{
    if (frame.error) {
        QColor errColor = ThemeManager::instance().color(
            ThemeManager::SemanticColor::Error);
        return QBrush(errColor.lighter(160));
    }
    if (frame.fd) {
        QColor fdColor = ThemeManager::instance().color(
            ThemeManager::SemanticColor::Success);
        return QBrush(fdColor.lighter(160));
    }
    if (frame.rtr) {
        return QBrush(palette().color(QPalette::Midlight));
    }
    if (frame.extended) {
        return QBrush(palette().color(QPalette::AlternateBase));
    }
    return QBrush(palette().color(QPalette::Base));
}
