/**
 * @file ModbusMasterPanelStats.cpp
 * @brief ModbusMasterPanel面板统计接口实现
 */
#include "protocol/modbus_master/ModbusMasterPanel.h"

/** @brief 重置面板统计计数器 */
void ModbusMasterPanel::resetPanelStatistics() {
    m_totalSendClicks = 0;
    m_totalPollToggles = 0;
    m_totalResponseUpdates = 0;
}
