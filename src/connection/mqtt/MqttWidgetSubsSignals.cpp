/**
 * @file MqttWidgetSubsSignals.cpp
 * @brief MQTT面板信号槽连接实现
 *
 * 从 MqttWidgetSubs.cpp 拆分而来，包含:
 *   - connectSignals(): 连接所有信号槽
 *     - 配置面板 → 连接/断开操作
 *     - 连接状态变更/消息接收
 *     - 发布按钮 → 发布操作
 *     - 订阅面板 → 订阅/退订操作
 *     - 重连定时器/统计刷新定时器
 *
 * UI布局初始化见 MqttWidgetSubs.cpp。
 */

#include "connection/mqtt/MqttWidget.h"
#include "connection/mqtt/MqttConnection.h"
#include "connection/mqtt/MqttConfigPanel.h"
#include "connection/mqtt/MqttSubscriptionPanel.h"
#include "connection/mqtt/MqttTopicModel.h"
#include "connection/interface/IConnection.h"

/** @brief 连接所有信号槽 */
void MqttWidget::connectSignals()
{
    /* 配置面板 → 连接操作 */
    connect(m_configPanel, &MqttConfigPanel::connectRequested,
            this, &MqttWidget::onConnectClicked);
    connect(m_configPanel, &MqttConfigPanel::disconnectRequested,
            this, [this]() {
        stopReconnect();
        m_connection->close();
    });

    /* 连接状态变更 */
    connect(m_connection, &MqttConnection::stateChanged,
            this, &MqttWidget::onConnectionStateChanged);
    connect(m_connection, &MqttConnection::messageReceived,
            this, &MqttWidget::onMessageReceived);

    /* 发布按钮 */
    connect(m_pubBtn, &QPushButton::clicked,
            this, &MqttWidget::onPublishClicked);

    /* 订阅面板 → 连接 */
    connect(m_subscriptionPanel, &MqttSubscriptionPanel::subscribeRequested,
            this, [this](const QString& topic, int qos) {
        if (m_connection->state() == ConnectionState::Connected) {
            m_connection->subscribe(topic, qos);
        }
        m_topicModel->addTopic(topic, qos);
    });
    connect(m_subscriptionPanel, &MqttSubscriptionPanel::unsubscribeRequested,
            this, [this](const QString& topic) {
        if (m_connection->state() == ConnectionState::Connected) {
            m_connection->unsubscribe(topic);
        }
        m_topicModel->removeTopic(topic);
    });

    /* 重连定时器 */
    connect(m_reconnectTimer, &QTimer::timeout,
            this, &MqttWidget::onReconnectTimer);

    /* 统计刷新定时器 */
    connect(m_statsRefreshTimer, &QTimer::timeout,
            this, &MqttWidget::updateStatistics);
}
