/**
 * @file WsConfigPanel.cpp
 * @brief WebSocket配置面板实现
 */

#include "connection/ws/WsConfigPanel.h"

#include <QFormLayout>

/** @brief 构造函数 - 初始化UI @param parent 父控件 */
WsConfigPanel::WsConfigPanel(QWidget* parent)
    : QWidget(parent)
{
    setObjectName("WsConfigPanel");
    setupUi();
    setupConnections();
}

/** @brief 获取当前配置参数 @return 配置键值对 */
QVariantMap WsConfigPanel::config() const
{
    QVariantMap cfg;
    if (m_typeCombo && m_urlEdit) {
        QString scheme = m_typeCombo->currentText().toLower();
        cfg["url"] = scheme + "://" + m_urlEdit->text();
    }
    if (m_protocolEdit) {
        cfg["protocol"] = m_protocolEdit->text();
    }
    return cfg;
}

/** @brief 连接按钮点击 - 切换连接/断开 */
void WsConfigPanel::onConnectClicked()
{
    if (!m_connected) {
        if (m_urlEdit->text().trimmed().isEmpty()) {
            m_statusLabel->setText(tr("请输入服务器地址"));
            return;
        }
        ++m_totalConnectAttempts;
        m_statusLabel->setText(tr("正在连接..."));
        emit connectRequested(config());
    } else {
        ++m_totalDisconnections;
        m_connected = false;
        m_statusLabel->setText(tr("已断开"));
        m_connectBtn->setText(tr("连接"));
        emit disconnectRequested();
    }
}

/** @brief 设置连接状态(由外部连接管理器调用) @param connected true=已连接 */
void WsConfigPanel::setConnected(bool connected)
{
    m_connected = connected;
    m_connectBtn->setText(connected ? tr("断开") : tr("连接"));
    m_statusLabel->setText(connected ? tr("已连接") : tr("未连接"));
}

/** @brief 初始化UI布局 */
void WsConfigPanel::setupUi()
{
    auto* layout = new QFormLayout(this);

    // 连接类型
    m_typeCombo = new QComboBox(this);
    m_typeCombo->setObjectName("wsTypeCombo");
    m_typeCombo->addItem("WS");
    m_typeCombo->addItem("WSS");
    layout->addRow(tr("协议类型:"), m_typeCombo);

    // URL输入
    m_urlEdit = new QLineEdit(this);
    m_urlEdit->setObjectName("wsUrlEdit");
    m_urlEdit->setPlaceholderText(tr("例如: 192.168.1.100:8080/ws"));
    layout->addRow(tr("服务器地址:"), m_urlEdit);

    // 子协议
    m_protocolEdit = new QLineEdit(this);
    m_protocolEdit->setObjectName("wsProtocolEdit");
    m_protocolEdit->setPlaceholderText(tr("子协议(可选)"));
    layout->addRow(tr("子协议:"), m_protocolEdit);

    // 连接按钮
    m_connectBtn = new QPushButton(tr("连接"), this);
    m_connectBtn->setObjectName("wsConnectBtn");
    layout->addRow(m_connectBtn);

    // 状态标签
    m_statusLabel = new QLabel(tr("未连接"), this);
    m_statusLabel->setObjectName("wsStatusLabel");
    layout->addRow(tr("状态:"), m_statusLabel);
}

/** @brief 初始化信号连接 */
void WsConfigPanel::setupConnections()
{
    connect(m_connectBtn, &QPushButton::clicked,
            this, &WsConfigPanel::onConnectClicked);

    // 协议类型变更时更新placeholder
    connect(m_typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int index) {
        Q_UNUSED(index)
        QString scheme = m_typeCombo->currentText().toLower();
        m_urlEdit->setPlaceholderText(
            tr("例如: 192.168.1.100:8080/ws (%1)").arg(scheme));
    });
}

/** @brief 保存WebSocket配置到QSettings @param settings QSettings对象 */
void WsConfigPanel::saveSettings(QSettings& settings) const
{
    settings.setValue(QStringLiteral("ws/type"), m_typeCombo->currentIndex());
    settings.setValue(QStringLiteral("ws/url"), m_urlEdit->text());
    settings.setValue(QStringLiteral("ws/protocol"), m_protocolEdit->text());
}

/** @brief 从QSettings加载WebSocket配置 @param settings QSettings对象 */
void WsConfigPanel::loadSettings(QSettings& settings)
{
    m_typeCombo->setCurrentIndex(
        settings.value(QStringLiteral("ws/type"), 0).toInt());
    m_urlEdit->setText(
        settings.value(QStringLiteral("ws/url")).toString());
    m_protocolEdit->setText(
        settings.value(QStringLiteral("ws/protocol")).toString());
}

/** @brief 重置所有统计计数器 */
void WsConfigPanel::resetStatistics()
{
    m_totalConnectAttempts = 0;
    m_totalDisconnections = 0;
}
