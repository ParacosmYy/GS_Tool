/**
 * @file MqttConfigPanel.cpp
 * @brief MQTT配置面板实现 — 服务器/认证/KeepAlive/Clean Session配置
 */

#include "connection/mqtt/MqttConfigPanel.h"
#include <QFormLayout>
#include <QHBoxLayout>

MqttConfigPanel::MqttConfigPanel(QWidget* parent)
    : QWidget(parent)
    , m_hostEdit(new QLineEdit(this))
    , m_portSpin(new QSpinBox(this))
    , m_clientIdEdit(new QLineEdit(this))
    , m_usernameEdit(new QLineEdit(this))
    , m_passwordEdit(new QLineEdit(this))
    , m_keepAliveSpin(new QSpinBox(this))
    , m_cleanSessionCheck(new QCheckBox(tr("Clean Session"), this))
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
    form->addRow(tr("Keep Alive:"), m_keepAliveSpin);
    form->addRow(m_cleanSessionCheck);

    auto btnLayout = new QHBoxLayout();
    btnLayout->addWidget(m_statusLabel, 1);
    btnLayout->addStretch();
    btnLayout->addWidget(m_connectBtn);
    form->addRow(btnLayout);

    /* 按钮点击 → 根据当前状态发送不同信号 */
    connect(m_connectBtn, &QPushButton::clicked, this, [this]() {
        if (m_connected) {
            emit disconnectRequested();
        } else {
            emit connectRequested();
        }
    });
}

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
