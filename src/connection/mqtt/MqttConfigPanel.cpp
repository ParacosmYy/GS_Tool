/**
 * @file MqttConfigPanel.cpp
 * @brief MQTT配置面板实现 — 服务器/认证/KeepAlive/Clean Session配置
 */

#include "connection/mqtt/MqttConfigPanel.h"
#include <QFormLayout>
#include <QHBoxLayout>

/** @brief 构造MQTT配置面板，初始化服务器/认证/KeepAlive/Clean Session配置UI @param parent 父控件 */
MqttConfigPanel::MqttConfigPanel(QWidget* parent)
    : QWidget(parent)
    , m_hostEdit(new QLineEdit(this))
    , m_portSpin(new QSpinBox(this))
    , m_clientIdEdit(new QLineEdit(this))
    , m_usernameEdit(new QLineEdit(this))
    , m_passwordEdit(new QLineEdit(this))
    , m_keepAliveSpin(new QSpinBox(this))
    , m_cleanSessionCheck(new QCheckBox(tr("清除会话"), this))
    , m_connectBtn(new QPushButton(tr("连接"), this))
    , m_statusLabel(new QLabel(tr("未连接"), this))
{
    setObjectName("MqttConfigPanel");

    /* 服务器地址 — 默认 broker.emqx.io */
    m_hostEdit->setObjectName("mqttHostEdit");
    m_hostEdit->setPlaceholderText(tr("例如 broker.emqx.io"));
    m_hostEdit->setText("broker.emqx.io");

    /* 端口 */
    m_portSpin->setObjectName("mqttPortSpin");
    m_portSpin->setRange(1, 65535);
    m_portSpin->setValue(1883);

    /* 客户端ID */
    m_clientIdEdit->setObjectName("mqttClientIdEdit");
    m_clientIdEdit->setPlaceholderText(tr("留空则自动生成"));

    /* 认证 */
    m_usernameEdit->setObjectName("mqttUsernameEdit");
    m_usernameEdit->setPlaceholderText(tr("可选"));
    m_passwordEdit->setObjectName("mqttPasswordEdit");
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setPlaceholderText(tr("可选"));

    /* KeepAlive */
    m_keepAliveSpin->setObjectName("mqttKeepAliveSpin");
    m_keepAliveSpin->setRange(10, 3600);
    m_keepAliveSpin->setValue(60);
    m_keepAliveSpin->setSuffix(tr(" 秒"));

    /* Clean Session — 默认开启 */
    m_cleanSessionCheck->setObjectName("mqttCleanSessionCheck");
    m_cleanSessionCheck->setChecked(true);

    /* 按钮/状态 */
    m_connectBtn->setObjectName("mqttConnectBtn");
    m_statusLabel->setObjectName("mqttStatusLabel");

    /* 布局 */
    auto form = new QFormLayout(this);
    form->addRow(tr("服务器地址:"), m_hostEdit);
    form->addRow(tr("端口:"), m_portSpin);
    form->addRow(tr("客户端ID:"), m_clientIdEdit);
    form->addRow(tr("用户名:"), m_usernameEdit);
    form->addRow(tr("密码:"), m_passwordEdit);
    form->addRow(tr("心跳间隔:"), m_keepAliveSpin);
    form->addRow(m_cleanSessionCheck);

    auto btnLayout = new QHBoxLayout();
    btnLayout->addWidget(m_statusLabel, 1);
    btnLayout->addStretch();
    btnLayout->addWidget(m_connectBtn);
    form->addRow(btnLayout);

    /* 按钮点击 → 根据当前状态发送不同信号 */
    connect(m_connectBtn, &QPushButton::clicked, this, [this]() {
        if (m_connected) {
            ++m_totalDisconnects;
            emit disconnectRequested();
        } else {
            ++m_totalConnectAttempts;
            emit connectRequested();
        }
    });

    /* 配置变更计数: 主机/端口/KeepAlive/Clean Session切换 */
    connect(m_hostEdit, &QLineEdit::editingFinished,
            this, [this]() { ++m_totalConfigChanges; ++m_totalBrokerChanges; });
    connect(m_portSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, [this]() { ++m_totalConfigChanges; });
    connect(m_keepAliveSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, [this]() { ++m_totalConfigChanges; });
    connect(m_cleanSessionCheck, &QCheckBox::checkStateChanged,
            this, [this]() { ++m_totalConfigChanges; });
}

/** @brief 获取当前MQTT连接配置参数 @return 包含host/port/clientId/username/password/keepAlive/cleanSession的配置Map */
QVariantMap MqttConfigPanel::config() const
{
    QVariantMap cfg;
    cfg["host"] = m_hostEdit->text().trimmed();
    cfg["port"] = m_portSpin->value();
    cfg["clientId"] = m_clientIdEdit->text().trimmed();
    cfg["username"] = m_usernameEdit->text().trimmed();
    cfg["password"] = m_passwordEdit->text();
    cfg["keepAlive"] = m_keepAliveSpin->value();
    cfg["cleanSession"] = m_cleanSessionCheck->isChecked();
    return cfg;
}

/** @brief 设置连接状态，更新按钮文本和控件可用性 @param connected true=已连接 */
void MqttConfigPanel::setConnected(bool connected)
{
    m_connected = connected;
    m_connectBtn->setText(connected ? tr("断开") : tr("连接"));
    m_statusLabel->setText(connected ? tr("已连接") : tr("未连接"));

    /* 连接中禁用配置项 */
    m_hostEdit->setEnabled(!connected);
    m_portSpin->setEnabled(!connected);
    m_clientIdEdit->setEnabled(!connected);
    m_usernameEdit->setEnabled(!connected);
    m_passwordEdit->setEnabled(!connected);
    m_keepAliveSpin->setEnabled(!connected);
    m_cleanSessionCheck->setEnabled(!connected);
}

/** @brief 保存MQTT配置到QSettings(不含密码) @param settings QSettings对象 */
void MqttConfigPanel::saveSettings(QSettings& settings) const
{
    settings.setValue(QStringLiteral("mqtt/host"), m_hostEdit->text());
    settings.setValue(QStringLiteral("mqtt/port"), m_portSpin->value());
    settings.setValue(QStringLiteral("mqtt/clientId"), m_clientIdEdit->text());
    settings.setValue(QStringLiteral("mqtt/username"), m_usernameEdit->text());
    settings.setValue(QStringLiteral("mqtt/keepAlive"), m_keepAliveSpin->value());
    settings.setValue(QStringLiteral("mqtt/cleanSession"),
                      m_cleanSessionCheck->isChecked());
    /* 不保存密码到明文设置 — 安全考虑 */
}

/** @brief 从QSettings加载MQTT配置 @param settings QSettings对象 */
void MqttConfigPanel::loadSettings(QSettings& settings)
{
    m_hostEdit->setText(
        settings.value(QStringLiteral("mqtt/host"),
                       QStringLiteral("broker.emqx.io")).toString());
    m_portSpin->setValue(
        settings.value(QStringLiteral("mqtt/port"), 1883).toInt());
    m_clientIdEdit->setText(
        settings.value(QStringLiteral("mqtt/clientId")).toString());
    m_usernameEdit->setText(
        settings.value(QStringLiteral("mqtt/username")).toString());
    m_keepAliveSpin->setValue(
        settings.value(QStringLiteral("mqtt/keepAlive"), 60).toInt());
    m_cleanSessionCheck->setChecked(
        settings.value(QStringLiteral("mqtt/cleanSession"), true).toBool());
}

/** @brief 重置所有统计计数器 */
void MqttConfigPanel::resetStatistics()
{
    m_totalConnectAttempts = 0;
    m_totalConfigChanges = 0;
    m_totalDisconnects = 0;
    m_totalBrokerChanges = 0;
}
