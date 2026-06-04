/**
 * @file MqttWidgetSlots.cpp
 * @brief MQTT面板槽函数实现 — 事件回调+重连+统计更新
 *
 * 从MqttWidget.cpp拆分，负责:
 *   1. 连接/发布按钮回调(onConnectClicked/onPublishClicked)
 *   2. 连接状态变更处理(onConnectionStateChanged)
 *   3. 消息接收路由(onMessageReceived)
 *   4. 自动重连定时器(onReconnectTimer/startReconnect/stopReconnect)
 *   5. 统计面板刷新(updateStatistics)
 *   6. 字节格式化(formatBytes)
 */

#include "connection/mqtt/MqttWidget.h"
#include "connection/mqtt/MqttConnection.h"
#include "connection/mqtt/MqttConfigPanel.h"
#include "connection/mqtt/MqttTopicModel.h"
#include "core/theme/ThemeManager.h"

/** @brief 连接按钮点击，应用配置并打开连接 */
void MqttWidget::onConnectClicked()
{
    stopReconnect();
    m_connection->configure(m_configPanel->config());
    m_connection->open();
}

/** @brief 发布按钮点击，发送消息到指定主题 */
void MqttWidget::onPublishClicked()
{
    const QString topic = m_pubTopicEdit->text().trimmed();
    if (topic.isEmpty()) return;

    const QByteArray payload = m_pubPayloadEdit->toPlainText().toUtf8();
    const int qos = m_pubQosCombo->currentData().toInt();

    bool success = false;
    if (m_connection->state() == ConnectionState::Connected) {
        success = m_connection->publish(topic, payload, qos);
    } else {
        /* 断线时入队 */
        success = m_connection->enqueueMessage(topic, payload, qos);
    }

    if (success) {
        ++m_totalPublishOps;
        emit messagePublished(topic, payload);
    }
}

/** @brief 连接状态变更回调，更新UI指示和自动重连 */
void MqttWidget::onConnectionStateChanged(ConnectionState state)
{
    using SC = ThemeManager::SemanticColor;
    switch (state) {
    case ConnectionState::Connected:
        m_statusIndicator->setStyleSheet(
            QStringLiteral("QLabel#mqttStatusIndicator {"
                            " background-color: %1;"
                            " border-radius: 7px;"
                            " }")
                .arg(ThemeManager::instance().color(SC::Success).name()));
        m_statusText->setText(tr("已连接 - %1").arg(m_connection->name()));
        m_configPanel->setConnected(true);
        ++m_totalSubscriptions;
        stopReconnect();
        break;
    case ConnectionState::Connecting:
        m_statusIndicator->setStyleSheet(
            QStringLiteral("QLabel#mqttStatusIndicator {"
                            " background-color: %1;"
                            " border-radius: 7px;"
                            " }")
                .arg(ThemeManager::instance().color(SC::Warning).name()));
        m_statusText->setText(tr("连接中..."));
        break;
    case ConnectionState::Disconnected:
        m_statusIndicator->setStyleSheet(
            QStringLiteral("QLabel#mqttStatusIndicator {"
                            " background-color: %1;"
                            " border-radius: 7px;"
                            " }")
                .arg(ThemeManager::instance().color(SC::TextMuted).name()));
        m_statusText->setText(tr("未连接"));
        m_configPanel->setConnected(false);
        ++m_totalUnsubscriptions;
        if (m_autoReconnect) startReconnect();
        break;
    case ConnectionState::Error:
        m_statusIndicator->setStyleSheet(
            QStringLiteral("QLabel#mqttStatusIndicator {"
                            " background-color: %1;"
                            " border-radius: 7px;"
                            " }")
                .arg(ThemeManager::instance().color(SC::Error).name()));
        m_statusText->setText(tr("连接错误"));
        m_configPanel->setConnected(false);
        if (m_autoReconnect) startReconnect();
        break;
    }
    updateStatistics();
}

/** @brief 接收到MQTT消息，路由到主题模型 @param topic 消息主题 @param payload 消息负载 */
void MqttWidget::onMessageReceived(const QString& topic, const QByteArray& payload)
{
    ++m_totalMessageDisplays;
    ++m_totalTopicFilters;
    m_topicModel->routeMessage(topic, payload);
}

/** @brief 自动重连定时器触发 */
void MqttWidget::onReconnectTimer()
{
    ++m_totalReconnectAttempts;
    m_connection->configure(m_configPanel->config());
    m_connection->open();
}

/** @brief 更新统计面板各标签数值 */
void MqttWidget::updateStatistics()
{
    m_statPublished->setText(QString::number(m_connection->totalPublishes()));
    m_statReceived->setText(QString::number(m_connection->totalReceived()));
    m_statQos0->setText(QString::number(m_connection->qos0Count()));
    m_statQos1->setText(QString::number(m_connection->qos1Count()));
    m_statQos2->setText(QString::number(m_connection->qos2Count()));
    m_statBytesSent->setText(formatBytes(m_connection->totalBytesSent()));
    m_statBytesReceived->setText(formatBytes(m_connection->totalBytesReceived()));
    m_statQueueSize->setText(QString::number(m_connection->pendingQueueSize()));
    m_statKeepAlive->setText(QString::number(m_connection->keepAliveSent()));
    m_statConnAttempts->setText(QString::number(m_connection->connectionAttempts()));

    const QDateTime last = m_connection->lastConnectTime();
    m_statLastConnect->setText(last.isValid()
        ? last.toString(QStringLiteral("yyyy-MM-dd HH:mm:ss"))
        : QStringLiteral("-"));
}

/** @brief 启动自动重连计时器 */
void MqttWidget::startReconnect()
{
    if (!m_reconnectTimer->isActive()) {
        m_reconnectTimer->start();
    }
}

/** @brief 停止自动重连计时器 */
void MqttWidget::stopReconnect()
{
    m_reconnectTimer->stop();
}

/** @brief 格式化字节大小为人类可读字符串 @param bytes 字节数 @return 如"1.23 KB" */
QString MqttWidget::formatBytes(quint64 bytes) const
{
    if (bytes < 1024) return tr("%1 B").arg(bytes);
    if (bytes < 1024 * 1024) return tr("%1 KB").arg(bytes / 1024.0, 0, 'f', 2);
    return tr("%1 MB").arg(bytes / (1024.0 * 1024.0), 0, 'f', 2);
}
