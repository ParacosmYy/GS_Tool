/**
 * @file NetworkTopologyWidgetStats.cpp
 * @brief NetworkTopologyWidget 统计接口实现
 */

#include "utils/network/NetworkTopologyWidget.h"

// ============================================================================
// 统计接口
// ============================================================================

/** @brief 获取累计重绘次数 @return 计数 */
quint64 NetworkTopologyWidget::totalRepaints() const { return m_totalRepaints; }

/** @brief 获取累计设备点击次数 @return 计数 */
quint64 NetworkTopologyWidget::totalDeviceClicks() const { return m_totalDeviceClicks; }

/** @brief 重置拓扑组件统计计数器(重绘次数/设备点击次数) */
void NetworkTopologyWidget::resetStatistics() {
    m_totalRepaints = 0;
    m_totalDeviceClicks = 0;
}
