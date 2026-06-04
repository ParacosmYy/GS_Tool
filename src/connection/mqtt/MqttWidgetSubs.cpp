/**
 * @file MqttWidgetSubs.cpp
 * @brief MQTT面板UI初始化与信号连接实现
 *
 * 从MqttWidget.cpp拆分，负责:
 *   1. 整体UI布局初始化(setupUi) — 状态栏/配置/发布/订阅/主题树/统计
 *   2. 信号槽连接(connectSignals) — 配置面板/发布/订阅/重连/统计刷新
 */

#include "connection/mqtt/MqttWidget.h"
#include "connection/mqtt/MqttConnection.h"
#include "connection/mqtt/MqttConfigPanel.h"
#include "connection/mqtt/MqttSubscriptionPanel.h"
#include "connection/mqtt/MqttTopicModel.h"
#include "connection/interface/IConnection.h"
#include "core/theme/ThemeManager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QFormLayout>

// ============================================================
// UI初始化
// ============================================================

/** @brief 初始化整体UI布局 */
void MqttWidget::setupUi()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(4, 4, 4, 4);
    mainLayout->setSpacing(4);

    /* ---- 顶部: 连接状态栏 ---- */
    auto* statusLayout = new QHBoxLayout();
    statusLayout->setSpacing(8);

    m_statusIndicator = new QLabel(this);
    m_statusIndicator->setObjectName("mqttStatusIndicator");
    m_statusIndicator->setFixedSize(14, 14);
    m_statusIndicator->setStyleSheet(
        QStringLiteral("QLabel#mqttStatusIndicator {"
                        " background-color: %1;"
                        " border-radius: 7px;"
                        " }")
            .arg(ThemeManager::instance().color(ThemeManager::SemanticColor::Error).name()));

    m_statusText = new QLabel(tr("未连接"), this);
    m_statusText->setObjectName("mqttStatusText");

    statusLayout->addWidget(m_statusIndicator);
    statusLayout->addWidget(m_statusText, 1);
    mainLayout->addLayout(statusLayout);

    /* ---- 中部: 分割器(左: 配置+发布 | 右: 订阅+主题树) ---- */
    auto* splitter = new QSplitter(Qt::Horizontal, this);
    splitter->setObjectName("mqttMainSplitter");

    /* 左侧面板 */
    auto* leftPanel = new QWidget(this);
    leftPanel->setObjectName("mqttLeftPanel");
    auto* leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(4);

    /* 配置面板 */
    auto* configGroup = new QGroupBox(tr("连接配置"), this);
    configGroup->setObjectName("mqttConfigGroup");
    auto* configLayout = new QVBoxLayout(configGroup);
    configLayout->addWidget(m_configPanel);
    leftLayout->addWidget(configGroup);

    /* 发布区域 */
    auto* pubGroup = new QGroupBox(tr("消息发布"), this);
    pubGroup->setObjectName("mqttPubGroup");
    auto* pubLayout = new QVBoxLayout(pubGroup);

    m_pubTopicEdit = new QLineEdit(this);
    m_pubTopicEdit->setObjectName("mqttPubTopicEdit");
    m_pubTopicEdit->setPlaceholderText(tr("发布主题，如 embeddebug/out"));

    m_pubQosCombo = new QComboBox(this);
    m_pubQosCombo->setObjectName("mqttPubQosCombo");
    m_pubQosCombo->addItem(tr("QoS 0"), 0);
    m_pubQosCombo->addItem(tr("QoS 1"), 1);
    m_pubQosCombo->addItem(tr("QoS 2"), 2);

    m_retainCheck = new QCheckBox(tr("保留"), this);
    m_retainCheck->setObjectName("mqttRetainCheck");

    auto* pubHeaderLayout = new QHBoxLayout();
    pubHeaderLayout->addWidget(m_pubTopicEdit, 1);
    pubHeaderLayout->addWidget(m_pubQosCombo);
    pubHeaderLayout->addWidget(m_retainCheck);
    pubLayout->addLayout(pubHeaderLayout);

    m_pubPayloadEdit = new QTextEdit(this);
    m_pubPayloadEdit->setObjectName("mqttPubPayloadEdit");
    m_pubPayloadEdit->setMaximumHeight(100);
    m_pubPayloadEdit->setPlaceholderText(tr("输入要发布的消息内容..."));
    pubLayout->addWidget(m_pubPayloadEdit);

    m_pubBtn = new QPushButton(tr("发布"), this);
    m_pubBtn->setObjectName("mqttPubBtn");
    pubLayout->addWidget(m_pubBtn);

    leftLayout->addWidget(pubGroup, 1);

    /* 右侧面板 */
    auto* rightPanel = new QWidget(this);
    rightPanel->setObjectName("mqttRightPanel");
    auto* rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(4);

    /* 订阅面板 */
    auto* subGroup = new QGroupBox(tr("主题订阅"), this);
    subGroup->setObjectName("mqttSubGroup");
    auto* subLayout = new QVBoxLayout(subGroup);
    subLayout->addWidget(m_subscriptionPanel);
    rightLayout->addWidget(subGroup);

    /* 主题树浏览器 */
    auto* topicGroup = new QGroupBox(tr("主题树"), this);
    topicGroup->setObjectName("mqttTopicGroup");
    auto* topicLayout = new QVBoxLayout(topicGroup);
    m_topicView = new QTreeView(this);
    m_topicView->setObjectName("mqttTopicView");
    m_topicView->setModel(m_topicModel);
    m_topicView->setHeaderHidden(false);
    m_topicView->setAlternatingRowColors(true);
    topicLayout->addWidget(m_topicView);
    rightLayout->addWidget(topicGroup, 1);

    splitter->addWidget(leftPanel);
    splitter->addWidget(rightPanel);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 1);
    mainLayout->addWidget(splitter, 1);

    /* ---- 底部: 统计面板 ---- */
    auto* statsGroup = new QGroupBox(tr("连接统计"), this);
    statsGroup->setObjectName("mqttStatsGroup");
    auto* statsLayout = new QFormLayout(statsGroup);

    m_statPublished = new QLabel(tr("0"), this);
    m_statPublished->setObjectName("mqttStatPublished");
    m_statReceived = new QLabel(tr("0"), this);
    m_statReceived->setObjectName("mqttStatReceived");
    m_statQos0 = new QLabel(tr("0"), this);
    m_statQos0->setObjectName("mqttStatQos0");
    m_statQos1 = new QLabel(tr("0"), this);
    m_statQos1->setObjectName("mqttStatQos1");
    m_statQos2 = new QLabel(tr("0"), this);
    m_statQos2->setObjectName("mqttStatQos2");
    m_statBytesSent = new QLabel(tr("0 B"), this);
    m_statBytesSent->setObjectName("mqttStatBytesSent");
    m_statBytesReceived = new QLabel(tr("0 B"), this);
    m_statBytesReceived->setObjectName("mqttStatBytesReceived");
    m_statQueueSize = new QLabel(tr("0"), this);
    m_statQueueSize->setObjectName("mqttStatQueueSize");
    m_statKeepAlive = new QLabel(tr("0"), this);
    m_statKeepAlive->setObjectName("mqttStatKeepAlive");
    m_statConnAttempts = new QLabel(tr("0"), this);
    m_statConnAttempts->setObjectName("mqttStatConnAttempts");
    m_statLastConnect = new QLabel(tr("-"), this);
    m_statLastConnect->setObjectName("mqttStatLastConnect");

    statsLayout->addRow(tr("发布消息:"), m_statPublished);
    statsLayout->addRow(tr("接收消息:"), m_statReceived);
    statsLayout->addRow(tr("QoS 0/1/2:"), m_statQos0);
    auto* qosRow2 = new QHBoxLayout();
    qosRow2->addWidget(m_statQos1);
    qosRow2->addWidget(m_statQos2);
    statsLayout->addRow(QString(), qosRow2);
    statsLayout->addRow(tr("发送/接收:"), m_statBytesSent);
    auto* bytesRow = new QHBoxLayout();
    bytesRow->addWidget(m_statBytesReceived);
    statsLayout->addRow(QString(), bytesRow);
    statsLayout->addRow(tr("队列深度:"), m_statQueueSize);
    statsLayout->addRow(tr("心跳次数:"), m_statKeepAlive);
    statsLayout->addRow(tr("连接尝试:"), m_statConnAttempts);
    statsLayout->addRow(tr("最后连接:"), m_statLastConnect);

    mainLayout->addWidget(statsGroup);
}

// ============================================================
// 信号连接
// ============================================================

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
