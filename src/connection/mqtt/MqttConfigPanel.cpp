/**
 * @file MqttConfigPanel.cpp
 * @brief MQTT配置面板实现
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
    , m_connectBtn(new QPushButton(tr("连接"), this))
{
    setObjectName("MqttConfigPanel");

    m_hostEdit->setPlaceholderText(tr("例如 broker.emqx.io"));
    m_portSpin->setRange(1, 65535);
    m_portSpin->setValue(1883);
    m_clientIdEdit->setPlaceholderText(tr("留空则自动生成"));
    m_usernameEdit->setPlaceholderText(tr("可选"));
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setPlaceholderText(tr("可选"));

    auto form = new QFormLayout(this);
    form->addRow(tr("服务器地址:"), m_hostEdit);
    form->addRow(tr("端口:"), m_portSpin);
    form->addRow(tr("客户端ID:"), m_clientIdEdit);
    form->addRow(tr("用户名:"), m_usernameEdit);
    form->addRow(tr("密码:"), m_passwordEdit);

    auto btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    btnLayout->addWidget(m_connectBtn);
    form->addRow(btnLayout);

    connect(m_connectBtn, &QPushButton::clicked,
            this, &MqttConfigPanel::connectRequested);
}

QVariantMap MqttConfigPanel::config() const
{
    QVariantMap cfg;
    cfg["host"] = m_hostEdit->text().trimmed();
    cfg["port"] = m_portSpin->value();
    cfg["clientId"] = m_clientIdEdit->text().trimmed();
    cfg["username"] = m_usernameEdit->text().trimmed();
    cfg["password"] = m_passwordEdit->text();
    return cfg;
}
