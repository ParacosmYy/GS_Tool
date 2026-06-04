/**
 * @file MqttWidget.cpp
 * @brief MQTT客户端整体面板实现 — 构造/析构/公共接口
 *
 * UI初始化见 MqttWidgetSubs.cpp
 * 槽函数+重连+统计见 MqttWidgetSlots.cpp
 */

#include "connection/mqtt/MqttWidget.h"
#include "connection/mqtt/MqttConfigPanel.h"
#include "connection/mqtt/MqttSubscriptionPanel.h"
#include "connection/mqtt/MqttTopicModel.h"

// ============================================================
// 构造 / 析构
// ============================================================

/** @brief 构造MQTT整体面板 @param connection MQTT连接对象 @param parent 父控件 */
MqttWidget::MqttWidget(MqttConnection* connection, QWidget* parent)
    : QWidget(parent)
    , m_connection(connection)
    , m_configPanel(new MqttConfigPanel(this))
    , m_subscriptionPanel(new MqttSubscriptionPanel(this))
    , m_topicModel(new MqttTopicModel(this))
    , m_reconnectTimer(new QTimer(this))
    , m_statsRefreshTimer(new QTimer(this))
{
    setObjectName("MqttWidget");
    Q_ASSERT(m_connection);

    setupUi();
    connectSignals();

    /* 统计面板每2秒刷新一次 */
    m_statsRefreshTimer->setInterval(2000);
    m_statsRefreshTimer->start();

    /* 重连定时器: 单次触发，5秒间隔 */
    m_reconnectTimer->setSingleShot(true);
    m_reconnectTimer->setInterval(m_reconnectInterval);
}

/** @brief 析构函数，停止定时器 */
MqttWidget::~MqttWidget()
{
    m_statsRefreshTimer->stop();
    m_reconnectTimer->stop();
}

// ============================================================
// 公共接口
// ============================================================

/** @brief 获取内部MQTT连接对象 @return 连接指针 */
MqttConnection* MqttWidget::connection() const { return m_connection; }

/** @brief 获取主题模型 @return 模型指针 */
MqttTopicModel* MqttWidget::topicModel() const { return m_topicModel; }

/** @brief 重置UI层统计计数器 */
void MqttWidget::resetWidgetStatistics()
{
    m_totalReconnectAttempts = 0;
    m_totalPublishOps = 0;
    m_totalSubscriptions = 0;
    m_totalUnsubscriptions = 0;
    m_totalMessageDisplays = 0;
    m_totalTopicFilters = 0;
}
