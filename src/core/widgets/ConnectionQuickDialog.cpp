/**
 * @file ConnectionQuickDialog.cpp
 * @brief 快速连接对话框实现 - 连接类型选择、默认值和参数收集
 */

#include "core/widgets/ConnectionQuickDialog.h"

#include "core/connect/ConnectionPresetBuilder.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>

/** @brief 构造快速连接对话框，初始化UI和默认连接类型 */
ConnectionQuickDialog::ConnectionQuickDialog(QWidget* parent)
    : QDialog(parent)
{
    setObjectName("connectionQuickDialog");
    setWindowTitle(tr("快速连接"));
    setModal(true);
    setMinimumWidth(520);
    setWindowFlag(Qt::WindowContextHelpButtonHint, false);

    setupUi();
    setupConnections();
    updateTypeUi();
}

/** @brief 构建对话框UI布局和控件 */
void ConnectionQuickDialog::setupUi()
{
    auto* form = new QFormLayout(this);
    form->setObjectName("connectionQuickDialogForm");
    form->setHorizontalSpacing(12);
    form->setVerticalSpacing(8);
    form->setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    auto* typeLabel = new QLabel(tr("连接类型:"), this);
    typeLabel->setObjectName("connectionQuickDialogTypeLabel");
    m_typeCombo = new QComboBox(this);
    m_typeCombo->setObjectName("connectionQuickDialogTypeCombo");
    m_typeCombo->addItem(tr("TCP客户端"), static_cast<int>(ConnectionType::TcpClient));
    m_typeCombo->addItem(tr("TCP服务端"), static_cast<int>(ConnectionType::TcpServer));
    m_typeCombo->addItem(tr("UDP"), static_cast<int>(ConnectionType::Udp));
    m_typeCombo->addItem(tr("WebSocket"), static_cast<int>(ConnectionType::WebSocket));
    m_typeCombo->addItem(tr("MQTT"), static_cast<int>(ConnectionType::Mqtt));
    m_typeCombo->addItem(tr("TLS"), static_cast<int>(ConnectionType::Tls));
    form->addRow(typeLabel, m_typeCombo);

    m_hostLabel = new QLabel(this);
    m_hostLabel->setObjectName("connectionQuickDialogHostLabel");
    m_hostEdit = new QLineEdit(this);
    m_hostEdit->setObjectName("connectionQuickDialogHostEdit");
    form->addRow(m_hostLabel, m_hostEdit);

    m_portLabel = new QLabel(this);
    m_portLabel->setObjectName("connectionQuickDialogPortLabel");
    m_portSpin = new QSpinBox(this);
    m_portSpin->setObjectName("connectionQuickDialogPortSpin");
    m_portSpin->setRange(0, 65535);
    form->addRow(m_portLabel, m_portSpin);

    m_urlLabel = new QLabel(this);
    m_urlLabel->setObjectName("connectionQuickDialogUrlLabel");
    m_urlEdit = new QLineEdit(this);
    m_urlEdit->setObjectName("connectionQuickDialogUrlEdit");
    form->addRow(m_urlLabel, m_urlEdit);

    m_localPortLabel = new QLabel(this);
    m_localPortLabel->setObjectName("connectionQuickDialogLocalPortLabel");
    m_localPortSpin = new QSpinBox(this);
    m_localPortSpin->setObjectName("connectionQuickDialogLocalPortSpin");
    m_localPortSpin->setRange(0, 65535);
    form->addRow(m_localPortLabel, m_localPortSpin);

    m_remoteHostLabel = new QLabel(this);
    m_remoteHostLabel->setObjectName("connectionQuickDialogRemoteHostLabel");
    m_remoteHostEdit = new QLineEdit(this);
    m_remoteHostEdit->setObjectName("connectionQuickDialogRemoteHostEdit");
    form->addRow(m_remoteHostLabel, m_remoteHostEdit);

    m_remotePortLabel = new QLabel(this);
    m_remotePortLabel->setObjectName("connectionQuickDialogRemotePortLabel");
    m_remotePortSpin = new QSpinBox(this);
    m_remotePortSpin->setObjectName("connectionQuickDialogRemotePortSpin");
    m_remotePortSpin->setRange(0, 65535);
    form->addRow(m_remotePortLabel, m_remotePortSpin);

    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    m_buttonBox->setObjectName("connectionQuickDialogButtons");
    if (auto* connectButton = m_buttonBox->button(QDialogButtonBox::Ok)) {
        connectButton->setText(tr("连接"));
        connectButton->setDefault(true);
    }
    if (auto* cancelButton = m_buttonBox->button(QDialogButtonBox::Cancel)) {
        cancelButton->setText(tr("取消"));
    }
    form->addRow(m_buttonBox);
}

/** @brief 初始化控件信号连接 */
void ConnectionQuickDialog::setupConnections()
{
    connect(m_typeCombo,
            static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, [this](int) { updateTypeUi(); });
    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

/** @brief 根据当前类型刷新默认值和字段可见性 */
void ConnectionQuickDialog::updateTypeUi()
{
    const ConnectionType type = selectedType();
    applyDefaults(type);

    const bool useHostPort = type == ConnectionType::TcpClient
        || type == ConnectionType::TcpServer
        || type == ConnectionType::Mqtt
        || type == ConnectionType::Tls;
    const bool useUrl = type == ConnectionType::WebSocket;
    const bool useUdp = type == ConnectionType::Udp;

    setRowVisible(m_hostLabel, m_hostEdit, useHostPort);
    setRowVisible(m_portLabel, m_portSpin, useHostPort);
    setRowVisible(m_urlLabel, m_urlEdit, useUrl);
    setRowVisible(m_localPortLabel, m_localPortSpin, useUdp);
    setRowVisible(m_remoteHostLabel, m_remoteHostEdit, useUdp);
    setRowVisible(m_remotePortLabel, m_remotePortSpin, useUdp);

    if (type == ConnectionType::TcpClient || type == ConnectionType::Mqtt || type == ConnectionType::Tls) {
        m_hostLabel->setText(tr("主机:"));
    } else if (type == ConnectionType::TcpServer) {
        m_hostLabel->setText(tr("监听地址:"));
    } else {
        m_hostLabel->setText(tr("地址:"));
    }

    m_portLabel->setText(tr("端口:"));
    m_urlLabel->setText(tr("URL:"));
    m_localPortLabel->setText(tr("本地端口:"));
    m_remoteHostLabel->setText(tr("远程主机:"));
    m_remotePortLabel->setText(tr("远程端口:"));
}

// 参数构建/类型适配方法(selectedType/params/applyDefaults/buildParams/updateTypeUi/setRowVisible)
// 见 ConnectionQuickDialogParams.cpp
