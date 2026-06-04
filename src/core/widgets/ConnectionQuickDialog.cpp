/**
 * @file ConnectionQuickDialog.cpp
 * @brief 快速连接对话框实现 - 连接类型选择、字段预设和参数收集
 */

#include "core/widgets/ConnectionQuickDialog.h"

#include "core/connect/ConnectionPresetBuilder.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QVBoxLayout>

/** @brief 构造快速连接对话框，初始化UI和默认连接类型 */
ConnectionQuickDialog::ConnectionQuickDialog(QWidget* parent)
    : QDialog(parent)
{
    setObjectName("connectionQuickDialog");
    setWindowTitle(tr("快速连接"));
    setModal(true);
    setMinimumWidth(560);
    setWindowFlag(Qt::WindowContextHelpButtonHint, false);

    setupUi();
    setupConnections();
    setSelectedType(ConnectionType::TcpClient);
}

/** @brief 构建对话框UI布局和控件 */
void ConnectionQuickDialog::setupUi()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 24, 24, 24);
    mainLayout->setSpacing(12);

    m_titleLabel = new QLabel(tr("快速连接"), this);
    m_titleLabel->setObjectName("connectionQuickDialogTitle");
    QFont titleFont = m_titleLabel->font();
    titleFont.setPointSize(titleFont.pointSize() + 2);
    titleFont.setBold(true);
    m_titleLabel->setFont(titleFont);

    m_descriptionLabel = new QLabel(
        tr("选择连接类型并填写必要参数。连接方式切换后会自动应用共享默认值。"),
        this);
    m_descriptionLabel->setObjectName("connectionQuickDialogDescription");
    m_descriptionLabel->setWordWrap(true);

    mainLayout->addWidget(m_titleLabel);
    mainLayout->addWidget(m_descriptionLabel);

    auto* form = new QFormLayout();
    form->setHorizontalSpacing(12);
    form->setVerticalSpacing(8);
    form->setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    form->setFormAlignment(Qt::AlignTop);

    m_typeLabel = new QLabel(tr("连接类型:"), this);
    m_typeLabel->setObjectName("connectionQuickDialogTypeLabel");
    m_typeCombo = new QComboBox(this);
    m_typeCombo->setObjectName("connectionQuickDialogTypeCombo");
    m_typeCombo->addItem(tr("TCP客户端"), static_cast<int>(ConnectionType::TcpClient));
    m_typeCombo->addItem(tr("TCP服务端"), static_cast<int>(ConnectionType::TcpServer));
    m_typeCombo->addItem(tr("UDP"), static_cast<int>(ConnectionType::Udp));
    m_typeCombo->addItem(tr("WebSocket"), static_cast<int>(ConnectionType::WebSocket));
    m_typeCombo->addItem(tr("MQTT"), static_cast<int>(ConnectionType::Mqtt));
    m_typeCombo->addItem(tr("TLS"), static_cast<int>(ConnectionType::Tls));
    form->addRow(m_typeLabel, m_typeCombo);

    m_hostLabel = new QLabel(this);
    m_hostLabel->setObjectName("connectionQuickDialogHostLabel");
    m_hostEdit = new QLineEdit(this);
    m_hostEdit->setObjectName("connectionQuickDialogHostEdit");
    form->addRow(m_hostLabel, m_hostEdit);

    m_portLabel = new QLabel(tr("端口:"), this);
    m_portLabel->setObjectName("connectionQuickDialogPortLabel");
    m_portSpin = new QSpinBox(this);
    m_portSpin->setObjectName("connectionQuickDialogPortSpin");
    m_portSpin->setRange(0, 65535);
    form->addRow(m_portLabel, m_portSpin);

    m_localPortLabel = new QLabel(tr("本地端口:"), this);
    m_localPortLabel->setObjectName("connectionQuickDialogLocalPortLabel");
    m_localPortSpin = new QSpinBox(this);
    m_localPortSpin->setObjectName("connectionQuickDialogLocalPortSpin");
    m_localPortSpin->setRange(0, 65535);
    form->addRow(m_localPortLabel, m_localPortSpin);

    m_extraLabel = new QLabel(tr("补充参数:"), this);
    m_extraLabel->setObjectName("connectionQuickDialogExtraLabel");
    m_extraEdit = new QLineEdit(this);
    m_extraEdit->setObjectName("connectionQuickDialogExtraEdit");
    form->addRow(m_extraLabel, m_extraEdit);

    m_clientIdLabel = new QLabel(tr("客户端ID:"), this);
    m_clientIdLabel->setObjectName("connectionQuickDialogClientIdLabel");
    m_clientIdEdit = new QLineEdit(this);
    m_clientIdEdit->setObjectName("connectionQuickDialogClientIdEdit");
    m_clientIdEdit->setPlaceholderText(tr("留空则自动生成"));
    form->addRow(m_clientIdLabel, m_clientIdEdit);

    m_usernameLabel = new QLabel(tr("用户名:"), this);
    m_usernameLabel->setObjectName("connectionQuickDialogUsernameLabel");
    m_usernameEdit = new QLineEdit(this);
    m_usernameEdit->setObjectName("connectionQuickDialogUsernameEdit");
    m_usernameEdit->setPlaceholderText(tr("可选"));
    form->addRow(m_usernameLabel, m_usernameEdit);

    m_passwordLabel = new QLabel(tr("密码:"), this);
    m_passwordLabel->setObjectName("connectionQuickDialogPasswordLabel");
    m_passwordEdit = new QLineEdit(this);
    m_passwordEdit->setObjectName("connectionQuickDialogPasswordEdit");
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setPlaceholderText(tr("可选"));
    form->addRow(m_passwordLabel, m_passwordEdit);

    m_keepAliveLabel = new QLabel(tr("心跳间隔:"), this);
    m_keepAliveLabel->setObjectName("connectionQuickDialogKeepAliveLabel");
    m_keepAliveSpin = new QSpinBox(this);
    m_keepAliveSpin->setObjectName("connectionQuickDialogKeepAliveSpin");
    m_keepAliveSpin->setRange(1, 3600);
    m_keepAliveSpin->setSuffix(tr(" 秒"));
    form->addRow(m_keepAliveLabel, m_keepAliveSpin);

    m_cleanSessionCheck = new QCheckBox(tr("清除会话"), this);
    m_cleanSessionCheck->setObjectName("connectionQuickDialogCleanSessionCheck");
    form->addRow(m_cleanSessionCheck);

    m_broadcastCheck = new QCheckBox(tr("广播模式"), this);
    m_broadcastCheck->setObjectName("connectionQuickDialogBroadcastCheck");
    form->addRow(m_broadcastCheck);

    m_peerVerifyCheck = new QCheckBox(tr("验证对端证书"), this);
    m_peerVerifyCheck->setObjectName("connectionQuickDialogPeerVerifyCheck");
    form->addRow(m_peerVerifyCheck);

    mainLayout->addLayout(form);

    m_hintLabel = new QLabel(this);
    m_hintLabel->setObjectName("connectionQuickDialogHintLabel");
    m_hintLabel->setWordWrap(true);
    mainLayout->addWidget(m_hintLabel);

    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    m_buttonBox->setObjectName("connectionQuickDialogButtons");
    if (auto* connectButton = m_buttonBox->button(QDialogButtonBox::Ok)) {
        connectButton->setText(tr("连接"));
        connectButton->setDefault(true);
    }
    if (auto* cancelButton = m_buttonBox->button(QDialogButtonBox::Cancel)) {
        cancelButton->setText(tr("取消"));
    }

    auto* buttonRow = new QHBoxLayout();
    buttonRow->addStretch(1);
    buttonRow->addWidget(m_buttonBox);
    mainLayout->addLayout(buttonRow);
}

/** @brief 初始化控件信号连接 */
void ConnectionQuickDialog::setupConnections()
{
    connect(m_typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int) { refreshTypeUi(); });

    connect(m_buttonBox, &QDialogButtonBox::accepted,
            this, [this]() {
                const QString error = validateCurrentInput();
                if (!error.isEmpty()) {
                    m_hintLabel->setText(error);
                    return;
                }
                accept();
            });
    connect(m_buttonBox, &QDialogButtonBox::rejected,
            this, &QDialog::reject);
}

/** @brief 根据当前选中的连接类型刷新预设值、标签文本和字段可见性 */
void ConnectionQuickDialog::refreshTypeUi()
{
    const ConnectionType type = selectedType();
    applyTypePreset(type);
    updateFieldLabels(type);
    updateFieldPlaceholders(type);
    updateFieldVisibility(type);
    m_hintLabel->setText(defaultHint(type));
}

/** @brief 设置指定连接类型并刷新界面 @param type 连接类型 */
void ConnectionQuickDialog::setSelectedType(ConnectionType type)
{
    const int index = m_typeCombo->findData(static_cast<int>(type));
    if (index >= 0) {
        QSignalBlocker blocker(m_typeCombo);
        m_typeCombo->setCurrentIndex(index);
        refreshTypeUi();
    }
}

/** @brief 获取当前选中的连接类型 @return 连接类型 */
ConnectionType ConnectionQuickDialog::selectedType() const
{
    const int typeValue = m_typeCombo->currentData().toInt();
    return static_cast<ConnectionType>(typeValue);
}

/** @brief 获取当前表单参数 @return 连接参数映射 */
QVariantMap ConnectionQuickDialog::params() const
{
    return buildParams(selectedType());
}

/** @brief 依据连接类型应用共享默认值 */
void ConnectionQuickDialog::applyTypePreset(ConnectionType type)
{
    const QVariantMap preset = ConnectionPresetBuilder::build(type);

    m_hostEdit->setText(preset.value("url",
                                     preset.value("host",
                                                  preset.value("listenAddress"))).toString());
    m_portSpin->setValue(preset.value("port", 0).toInt());
    m_localPortSpin->setValue(preset.value("localPort", 0).toInt());
    m_extraEdit->setText(preset.value("protocol").toString());
    m_clientIdEdit->clear();
    m_usernameEdit->clear();
    m_passwordEdit->clear();
    m_keepAliveSpin->setValue(60);
    m_cleanSessionCheck->setChecked(true);
    m_broadcastCheck->setChecked(false);
    m_peerVerifyCheck->setChecked(true);

    if (type == ConnectionType::TcpServer) {
        m_hostEdit->setText(QStringLiteral("0.0.0.0"));
        if (m_portSpin->value() == 0) {
            m_portSpin->setValue(8080);
        }
    } else if (type == ConnectionType::WebSocket) {
        if (m_hostEdit->text().trimmed().isEmpty()) {
            m_hostEdit->setText(QStringLiteral("ws://127.0.0.1:8080"));
        }
    } else if (type == ConnectionType::Mqtt) {
        if (m_portSpin->value() == 0) {
            m_portSpin->setValue(1883);
        }
        m_keepAliveSpin->setValue(60);
        m_cleanSessionCheck->setChecked(true);
    } else if (type == ConnectionType::Tls) {
        if (m_portSpin->value() == 0) {
            m_portSpin->setValue(443);
        }
    } else if (type == ConnectionType::Udp) {
        if (m_localPortSpin->value() == 0) {
            m_localPortSpin->setValue(8888);
        }
    }
}

/** @brief 更新字段显示/隐藏状态 @param type 当前连接类型 */
void ConnectionQuickDialog::updateFieldVisibility(ConnectionType type)
{
    const bool isTcpClient = type == ConnectionType::TcpClient;
    const bool isTcpServer = type == ConnectionType::TcpServer;
    const bool isUdp = type == ConnectionType::Udp;
    const bool isWebSocket = type == ConnectionType::WebSocket;
    const bool isMqtt = type == ConnectionType::Mqtt;
    const bool isTls = type == ConnectionType::Tls;

    setRowVisible(m_portLabel, m_portSpin, isTcpClient || isTcpServer || isUdp || isMqtt || isTls);
    setRowVisible(m_localPortLabel, m_localPortSpin, isUdp);
    setRowVisible(m_extraLabel, m_extraEdit, isWebSocket);
    setRowVisible(m_clientIdLabel, m_clientIdEdit, isMqtt);
    setRowVisible(m_usernameLabel, m_usernameEdit, isMqtt);
    setRowVisible(m_passwordLabel, m_passwordEdit, isMqtt);
    setRowVisible(m_keepAliveLabel, m_keepAliveSpin, isMqtt);
    m_cleanSessionCheck->setVisible(isMqtt);
    m_broadcastCheck->setVisible(isUdp);
    m_peerVerifyCheck->setVisible(isTls);
}

/** @brief 更新字段标签文本 @param type 当前连接类型 */
void ConnectionQuickDialog::updateFieldLabels(ConnectionType type)
{
    switch (type) {
    case ConnectionType::TcpClient:
        m_hostLabel->setText(tr("主机地址:"));
        break;
    case ConnectionType::TcpServer:
        m_hostLabel->setText(tr("监听地址:"));
        break;
    case ConnectionType::Udp:
        m_hostLabel->setText(tr("远程主机:"));
        break;
    case ConnectionType::WebSocket:
        m_hostLabel->setText(tr("URL:"));
        break;
    case ConnectionType::Mqtt:
    case ConnectionType::Tls:
        m_hostLabel->setText(tr("服务器地址:"));
        break;
    default:
        m_hostLabel->setText(tr("地址:"));
        break;
    }

    if (type == ConnectionType::WebSocket) {
        m_extraLabel->setText(tr("子协议:"));
    } else {
        m_extraLabel->setText(tr("补充参数:"));
    }

    if (type == ConnectionType::Udp) {
        m_portLabel->setText(tr("远程端口:"));
    } else if (type == ConnectionType::TcpServer) {
        m_portLabel->setText(tr("监听端口:"));
    } else {
        m_portLabel->setText(tr("端口:"));
    }
}

/** @brief 更新字段占位提示 @param type 当前连接类型 */
void ConnectionQuickDialog::updateFieldPlaceholders(ConnectionType type)
{
    switch (type) {
    case ConnectionType::TcpClient:
        m_hostEdit->setPlaceholderText(tr("例如 127.0.0.1"));
        break;
    case ConnectionType::TcpServer:
        m_hostEdit->setPlaceholderText(tr("例如 0.0.0.0"));
        break;
    case ConnectionType::Udp:
        m_hostEdit->setPlaceholderText(tr("例如 127.0.0.1"));
        break;
    case ConnectionType::WebSocket:
        m_hostEdit->setPlaceholderText(tr("例如 ws://127.0.0.1:8080"));
        m_extraEdit->setPlaceholderText(tr("可选子协议"));
        break;
    case ConnectionType::Mqtt:
        m_hostEdit->setPlaceholderText(tr("例如 broker.emqx.io"));
        break;
    case ConnectionType::Tls:
        m_hostEdit->setPlaceholderText(tr("例如 127.0.0.1"));
        break;
    default:
        break;
    }
}

/** @brief 校验当前输入是否满足最基础连接条件 @return 通过时返回空字符串，否则返回错误提示 */
QString ConnectionQuickDialog::validateCurrentInput() const
{
    const ConnectionType type = selectedType();
    const QString host = m_hostEdit->text().trimmed();

    if (type == ConnectionType::WebSocket) {
        if (host.isEmpty()) {
            return tr("请输入 WebSocket URL。");
        }
        return QString();
    }

    if (host.isEmpty()) {
        if (type == ConnectionType::TcpServer) {
            return tr("请输入监听地址。");
        }
        if (type == ConnectionType::Udp) {
            return tr("请输入远程主机地址。");
        }
        return tr("请输入主机地址。");
    }

    return QString();
}

/** @brief 按连接类型构建参数映射 @param type 连接类型 @return 可直接传给连接控制器的参数 */
QVariantMap ConnectionQuickDialog::buildParams(ConnectionType type) const
{
    QVariantMap params;

    switch (type) {
    case ConnectionType::TcpClient:
        params["mode"] = "client";
        params["host"] = m_hostEdit->text().trimmed();
        params["port"] = m_portSpin->value();
        break;
    case ConnectionType::TcpServer:
        params["mode"] = "server";
        params["listenAddress"] = m_hostEdit->text().trimmed();
        params["port"] = m_portSpin->value();
        break;
    case ConnectionType::Udp:
        params["localPort"] = m_localPortSpin->value();
        params["remoteHost"] = m_hostEdit->text().trimmed();
        params["remotePort"] = m_portSpin->value();
        params["broadcast"] = m_broadcastCheck->isChecked();
        break;
    case ConnectionType::WebSocket: {
        QString url = normalizeWebSocketUrl(m_hostEdit->text().trimmed());
        params["url"] = url;
        const QString protocol = m_extraEdit->text().trimmed();
        if (!protocol.isEmpty()) {
            params["protocol"] = protocol;
        }
        break;
    }
    case ConnectionType::Mqtt:
        params["host"] = m_hostEdit->text().trimmed();
        params["port"] = m_portSpin->value();
        params["clientId"] = m_clientIdEdit->text().trimmed();
        params["username"] = m_usernameEdit->text().trimmed();
        params["password"] = m_passwordEdit->text();
        params["keepAlive"] = m_keepAliveSpin->value();
        params["cleanSession"] = m_cleanSessionCheck->isChecked();
        break;
    case ConnectionType::Tls:
        params["host"] = m_hostEdit->text().trimmed();
        params["port"] = m_portSpin->value();
        params["peerVerify"] = m_peerVerifyCheck->isChecked();
        break;
    default:
        break;
    }

    return params;
}

/** @brief 生成当前类型的默认提示文本 @param type 连接类型 @return 提示文本 */
QString ConnectionQuickDialog::defaultHint(ConnectionType type)
{
    switch (type) {
    case ConnectionType::TcpClient:
        return tr("提示：TCP 客户端只需要主机地址和端口。");
    case ConnectionType::TcpServer:
        return tr("提示：TCP 服务端默认监听 0.0.0.0。");
    case ConnectionType::Udp:
        return tr("提示：UDP 需要远程主机、远程端口和本地端口。");
    case ConnectionType::WebSocket:
        return tr("提示：WebSocket 建议输入完整 URL，例如 ws://127.0.0.1:8080。");
    case ConnectionType::Mqtt:
        return tr("提示：MQTT 可选填写客户端 ID、用户名和密码。");
    case ConnectionType::Tls:
        return tr("提示：TLS 默认开启证书校验。");
    default:
        return QString();
    }
}

/** @brief 规范化 WebSocket URL，允许用户直接输入 host:port 的快速写法 @param text 用户输入 @return 规范化后的 URL */
QString ConnectionQuickDialog::normalizeWebSocketUrl(const QString& text)
{
    if (text.contains("://")) {
        return text;
    }
    return QStringLiteral("ws://%1").arg(text);
}

/** @brief 统一设置一行的显示/隐藏状态 @param label 标签控件 @param field 输入控件 @param visible 是否可见 */
void ConnectionQuickDialog::setRowVisible(QWidget* label, QWidget* field, bool visible)
{
    if (label) {
        label->setVisible(visible);
    }
    if (field) {
        field->setVisible(visible);
    }
}
