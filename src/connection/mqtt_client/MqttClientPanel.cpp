/**
 * @file MqttClientPanel.cpp
 * @brief MQTT客户端面板 — UI构建与交互逻辑实现
 *
 * 包含连接参数表单、订阅管理、发布表单、消息日志四个功能区域，
 * 通过MqttClientEngine实现完整的MQTT调试交互。
 */

#include "connection/mqtt_client/MqttClientPanel.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QDateTime>
#include <QScrollBar>

/** @brief 构造MQTT客户端面板 @param parent 父控件 */
MqttClientPanel::MqttClientPanel(QWidget* parent)
    : QWidget(parent)
    , m_engine(new MqttClientEngine(this))
    , m_totalPublishClicks(0)
    , m_totalSubscribeOps(0)
    , m_totalConnectClicks(0)
{
    setObjectName(QStringLiteral("mqttClientPanel"));
    setupUI();

    /* 引擎信号 → 面板槽 */
    connect(m_engine, &MqttClientEngine::connected,
            this, &MqttClientPanel::onEngineConnected);
    connect(m_engine, &MqttClientEngine::disconnected,
            this, &MqttClientPanel::onEngineDisconnected);
    connect(m_engine, &MqttClientEngine::messageReceived,
            this, &MqttClientPanel::onEngineMessageReceived);
    connect(m_engine, &MqttClientEngine::connectionError,
            this, &MqttClientPanel::onEngineError);
}

/** @brief 获取内部引擎 @return 引擎指针 */
MqttClientEngine* MqttClientPanel::engine() const { return m_engine; }

/** @brief 构建完整UI布局 */
void MqttClientPanel::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(6, 6, 6, 6);
    mainLayout->setSpacing(6);

    /* 顶部: 连接参数 */
    mainLayout->addWidget(createConnectionGroup());

    /* 中间: 订阅 + 发布 水平分栏 */
    auto* midSplitter = new QSplitter(Qt::Horizontal, this);
    midSplitter->setObjectName(QStringLiteral("mqttMidSplitter"));
    midSplitter->addWidget(createSubscribeGroup());
    midSplitter->addWidget(createPublishGroup());
    midSplitter->setStretchFactor(0, 1);
    midSplitter->setStretchFactor(1, 2);
    mainLayout->addWidget(midSplitter, 1);

    /* 底部: 消息日志 */
    mainLayout->addWidget(createLogGroup(), 2);
}

/** @brief 创建连接参数分组 @return 分组控件 */
QGroupBox* MqttClientPanel::createConnectionGroup()
{
    auto* group = new QGroupBox(tr("连接设置"), this);
    group->setObjectName(QStringLiteral("mqttConnectionGroup"));
    auto* layout = new QHBoxLayout(group);
    layout->setSpacing(8);

    /* Broker */
    m_brokerEdit = new QLineEdit(this);
    m_brokerEdit->setObjectName(QStringLiteral("mqttBrokerEdit"));
    m_brokerEdit->setPlaceholderText(tr("Broker地址 (如 127.0.0.1)"));
    layout->addWidget(new QLabel(tr("Broker:")), 0);
    layout->addWidget(m_brokerEdit, 3);

    /* Port */
    m_portSpin = new QSpinBox(this);
    m_portSpin->setObjectName(QStringLiteral("mqttPortSpin"));
    m_portSpin->setRange(1, 65535);
    m_portSpin->setValue(1883);
    layout->addWidget(new QLabel(tr("端口:")), 0);
    layout->addWidget(m_portSpin, 1);

    /* Client ID */
    m_clientIdEdit = new QLineEdit(this);
    m_clientIdEdit->setObjectName(QStringLiteral("mqttClientIdEdit"));
    m_clientIdEdit->setPlaceholderText(tr("客户端ID (留空自动生成)"));
    layout->addWidget(new QLabel(tr("Client ID:")), 0);
    layout->addWidget(m_clientIdEdit, 2);

    /* Username */
    m_usernameEdit = new QLineEdit(this);
    m_usernameEdit->setObjectName(QStringLiteral("mqttUsernameEdit"));
    m_usernameEdit->setPlaceholderText(tr("用户名 (可选)"));
    layout->addWidget(new QLabel(tr("用户:")), 0);
    layout->addWidget(m_usernameEdit, 1);

    /* Password */
    m_passwordEdit = new QLineEdit(this);
    m_passwordEdit->setObjectName(QStringLiteral("mqttPasswordEdit"));
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setPlaceholderText(tr("密码 (可选)"));
    layout->addWidget(new QLabel(tr("密码:")), 0);
    layout->addWidget(m_passwordEdit, 1);

    /* TLS */
    m_tlsCheck = new QCheckBox(tr("TLS"), this);
    m_tlsCheck->setObjectName(QStringLiteral("mqttTlsCheck"));
    layout->addWidget(m_tlsCheck);

    /* KeepAlive */
    m_keepAliveSpin = new QSpinBox(this);
    m_keepAliveSpin->setObjectName(QStringLiteral("mqttKeepAliveSpin"));
    m_keepAliveSpin->setRange(1, 3600);
    m_keepAliveSpin->setValue(60);
    m_keepAliveSpin->setSuffix(tr(" 秒"));
    layout->addWidget(new QLabel(tr("心跳:")), 0);
    layout->addWidget(m_keepAliveSpin, 1);

    /* 按钮 */
    m_connectBtn = new QPushButton(tr("连接"), this);
    m_connectBtn->setObjectName(QStringLiteral("mqttConnectBtn"));
    connect(m_connectBtn, &QPushButton::clicked,
            this, &MqttClientPanel::onConnectClicked);
    layout->addWidget(m_connectBtn);

    m_disconnectBtn = new QPushButton(tr("断开"), this);
    m_disconnectBtn->setObjectName(QStringLiteral("mqttDisconnectBtn"));
    m_disconnectBtn->setEnabled(false);
    connect(m_disconnectBtn, &QPushButton::clicked,
            this, &MqttClientPanel::onDisconnectClicked);
    layout->addWidget(m_disconnectBtn);

    /* 状态 */
    m_statusLabel = new QLabel(tr("未连接"), this);
    m_statusLabel->setObjectName(QStringLiteral("mqttStatusLabel"));
    layout->addWidget(m_statusLabel);

    return group;
}

/** @brief 创建订阅管理分组 @return 分组控件 */
QGroupBox* MqttClientPanel::createSubscribeGroup()
{
    auto* group = new QGroupBox(tr("订阅管理"), this);
    group->setObjectName(QStringLiteral("mqttSubscribeGroup"));
    auto* layout = new QVBoxLayout(group);

    /* 输入行 */
    auto* inputRow = new QHBoxLayout();
    m_subTopicEdit = new QLineEdit(this);
    m_subTopicEdit->setObjectName(QStringLiteral("mqttSubTopicEdit"));
    m_subTopicEdit->setPlaceholderText(tr("主题过滤器 (如 sensor/#)"));
    inputRow->addWidget(m_subTopicEdit, 3);

    m_subQosCombo = new QComboBox(this);
    m_subQosCombo->setObjectName(QStringLiteral("mqttSubQosCombo"));
    m_subQosCombo->addItem(QStringLiteral("QoS 0"), static_cast<int>(MqttQos::QoS0));
    m_subQosCombo->addItem(QStringLiteral("QoS 1"), static_cast<int>(MqttQos::QoS1));
    m_subQosCombo->addItem(QStringLiteral("QoS 2"), static_cast<int>(MqttQos::QoS2));
    inputRow->addWidget(m_subQosCombo, 1);

    m_subBtn = new QPushButton(tr("订阅"), this);
    m_subBtn->setObjectName(QStringLiteral("mqttSubBtn"));
    connect(m_subBtn, &QPushButton::clicked,
            this, &MqttClientPanel::onSubscribeClicked);
    inputRow->addWidget(m_subBtn);
    layout->addLayout(inputRow);

    /* 已订阅列表 */
    m_subListWidget = new QListWidget(this);
    m_subListWidget->setObjectName(QStringLiteral("mqttSubListWidget"));
    layout->addWidget(m_subListWidget, 1);

    /* 退订按钮 */
    m_unsubBtn = new QPushButton(tr("取消订阅"), this);
    m_unsubBtn->setObjectName(QStringLiteral("mqttUnsubBtn"));
    connect(m_unsubBtn, &QPushButton::clicked,
            this, &MqttClientPanel::onUnsubscribeClicked);
    layout->addWidget(m_unsubBtn);

    return group;
}

/** @brief 创建发布表单分组 @return 分组控件 */
QGroupBox* MqttClientPanel::createPublishGroup()
{
    auto* group = new QGroupBox(tr("消息发布"), this);
    group->setObjectName(QStringLiteral("mqttPublishGroup"));
    auto* layout = new QVBoxLayout(group);

    /* Topic行 */
    auto* topicRow = new QHBoxLayout();
    topicRow->addWidget(new QLabel(tr("主题:")));
    m_pubTopicEdit = new QLineEdit(this);
    m_pubTopicEdit->setObjectName(QStringLiteral("mqttPubTopicEdit"));
    m_pubTopicEdit->setPlaceholderText(tr("发布目标主题"));
    topicRow->addWidget(m_pubTopicEdit, 3);

    m_pubQosCombo = new QComboBox(this);
    m_pubQosCombo->setObjectName(QStringLiteral("mqttPubQosCombo"));
    m_pubQosCombo->addItem(QStringLiteral("QoS 0"), static_cast<int>(MqttQos::QoS0));
    m_pubQosCombo->addItem(QStringLiteral("QoS 1"), static_cast<int>(MqttQos::QoS1));
    m_pubQosCombo->addItem(QStringLiteral("QoS 2"), static_cast<int>(MqttQos::QoS2));
    topicRow->addWidget(m_pubQosCombo, 1);

    m_pubRetainCheck = new QCheckBox(tr("Retain"), this);
    m_pubRetainCheck->setObjectName(QStringLiteral("mqttPubRetainCheck"));
    topicRow->addWidget(m_pubRetainCheck);
    layout->addLayout(topicRow);

    /* Payload */
    layout->addWidget(new QLabel(tr("负载:")));
    m_pubPayloadEdit = new QTextEdit(this);
    m_pubPayloadEdit->setObjectName(QStringLiteral("mqttPubPayloadEdit"));
    m_pubPayloadEdit->setMaximumHeight(100);
    m_pubPayloadEdit->setPlaceholderText(tr("输入消息负载 (支持HEX和文本)"));
    layout->addWidget(m_pubPayloadEdit, 1);

    /* 发布按钮 */
    m_pubBtn = new QPushButton(tr("发布"), this);
    m_pubBtn->setObjectName(QStringLiteral("mqttPubBtn"));
    connect(m_pubBtn, &QPushButton::clicked,
            this, &MqttClientPanel::onPublishClicked);
    layout->addWidget(m_pubBtn);

    return group;
}

/** @brief 创建消息日志分组 @return 分组控件 */
QGroupBox* MqttClientPanel::createLogGroup()
{
    auto* group = new QGroupBox(tr("消息日志"), this);
    group->setObjectName(QStringLiteral("mqttLogGroup"));
    auto* layout = new QVBoxLayout(group);

    m_logEdit = new QTextEdit(this);
    m_logEdit->setObjectName(QStringLiteral("mqttLogEdit"));
    m_logEdit->setReadOnly(true);
    m_logEdit->setPlaceholderText(tr("收到的MQTT消息将在此显示..."));
    layout->addWidget(m_logEdit, 1);

    m_clearLogBtn = new QPushButton(tr("清空日志"), this);
    m_clearLogBtn->setObjectName(QStringLiteral("mqttClearLogBtn"));
    connect(m_clearLogBtn, &QPushButton::clicked,
            this, &MqttClientPanel::onClearLogClicked);
    layout->addWidget(m_clearLogBtn);

    return group;
}

// ── 槽函数 ──────────────────────────────────────────────────

/** @brief 连接按钮点击处理 */
void MqttClientPanel::onConnectClicked()
{
    ++m_totalConnectClicks;
    MqttConnectionParams params;
    params.broker = m_brokerEdit->text().trimmed();
    params.port = static_cast<quint16>(m_portSpin->value());
    params.clientId = m_clientIdEdit->text().trimmed();
    params.username = m_usernameEdit->text().trimmed();
    params.password = m_passwordEdit->text();
    params.useTls = m_tlsCheck->isChecked();
    params.keepAlive = m_keepAliveSpin->value();
    m_engine->connectToBroker(params);
    m_statusLabel->setText(tr("正在连接..."));
}

/** @brief 断开按钮点击处理 */
void MqttClientPanel::onDisconnectClicked()
{
    m_engine->disconnectFromBroker();
}

/** @brief 订阅按钮点击处理 */
void MqttClientPanel::onSubscribeClicked()
{
    QString topic = m_subTopicEdit->text().trimmed();
    if (topic.isEmpty()) {
        return;
    }
    MqttQos qos = static_cast<MqttQos>(m_subQosCombo->currentData().toInt());
    if (m_engine->subscribe(topic, qos)) {
        ++m_totalSubscribeOps;
        m_subListWidget->addItem(QStringLiteral("%1 (QoS %2)")
            .arg(topic, QString::number(static_cast<int>(qos))));
        m_subTopicEdit->clear();
    }
}

/** @brief 退订按钮点击处理 */
void MqttClientPanel::onUnsubscribeClicked()
{
    int row = m_subListWidget->currentRow();
    if (row < 0) {
        return;
    }
    QString entry = m_subListWidget->item(row)->text();
    QString topic = entry.left(entry.indexOf(QLatin1String(" (")));
    m_engine->unsubscribe(topic);
    delete m_subListWidget->takeItem(row);
}

/** @brief 发布按钮点击处理 */
void MqttClientPanel::onPublishClicked()
{
    QString topic = m_pubTopicEdit->text().trimmed();
    if (topic.isEmpty()) {
        return;
    }
    QByteArray payload = m_pubPayloadEdit->toPlainText().toUtf8();
    MqttQos qos = static_cast<MqttQos>(m_pubQosCombo->currentData().toInt());
    bool retain = m_pubRetainCheck->isChecked();
    m_engine->publish(topic, payload, qos, retain);
    ++m_totalPublishClicks;
}

/** @brief 清空日志按钮处理 */
void MqttClientPanel::onClearLogClicked()
{
    m_logEdit->clear();
}

/** @brief 引擎连接成功回调 */
void MqttClientPanel::onEngineConnected()
{
    updateConnectionState(true);
    m_statusLabel->setText(tr("已连接"));
}

/** @brief 引擎断开连接回调 */
void MqttClientPanel::onEngineDisconnected()
{
    updateConnectionState(false);
    m_statusLabel->setText(tr("已断开"));
}

/** @brief 引擎收到消息回调 @param msg 消息 */
void MqttClientPanel::onEngineMessageReceived(const MqttMessage& msg)
{
    QString timestamp = msg.timestamp.toString(QStringLiteral("HH:mm:ss.zzz"));
    QString qosStr = QString::number(static_cast<int>(msg.qos));
    QString retainStr = msg.retained ? QStringLiteral(" [R]") : QString();
    QString line = QStringLiteral("[%1] %2 (QoS %3%4): %5")
        .arg(timestamp, msg.topic, qosStr, retainStr,
             QString::fromUtf8(msg.payload));
    m_logEdit->append(line);
    /* 自动滚动到底部 */
    QScrollBar* bar = m_logEdit->verticalScrollBar();
    bar->setValue(bar->maximum());
}

/** @brief 引擎连接错误回调 @param error 错误信息 */
void MqttClientPanel::onEngineError(const QString& error)
{
    m_statusLabel->setText(tr("错误: %1").arg(error));
    m_logEdit->append(QStringLiteral("[%1] ERROR: %2")
        .arg(QDateTime::currentDateTime().toString(QStringLiteral("HH:mm:ss")),
             error));
}

/** @brief 更新连接状态UI @param connected 是否已连接 */
void MqttClientPanel::updateConnectionState(bool connected)
{
    m_connectBtn->setEnabled(!connected);
    m_disconnectBtn->setEnabled(connected);
    m_brokerEdit->setEnabled(!connected);
    m_portSpin->setEnabled(!connected);
    m_clientIdEdit->setEnabled(!connected);
    m_usernameEdit->setEnabled(!connected);
    m_passwordEdit->setEnabled(!connected);
    m_tlsCheck->setEnabled(!connected);
    m_keepAliveSpin->setEnabled(!connected);
}
