/**
 * @file MqttClientPanelStats.cpp
 * @brief MQTT客户端面板 — 统计查询与重置接口实现
 *
 * 从 MqttClientPanel.cpp 拆分而来，包含面板级统计 getter 和 resetStats 方法。
 */

#include "connection/mqtt_client/MqttClientPanel.h"

/** @brief 重置面板统计计数器 */
void MqttClientPanel::resetStats()
{
    m_totalPublishClicks = 0;
    m_totalSubscribeOps = 0;
    m_totalConnectClicks = 0;
}
