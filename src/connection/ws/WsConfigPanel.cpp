/**
 * @file WsConfigPanel.cpp
 * @brief WebSocket配置面板实现 - 骨架
 */

#include "connection/ws/WsConfigPanel.h"

/**
 * @brief 构造函数 - 初始化UI
 * @param parent 父控件
 */
WsConfigPanel::WsConfigPanel(QWidget* parent)
    : QWidget(parent)
{
    setObjectName("WsConfigPanel");
    setupUi();
    setupConnections();
}

/**
 * @brief 获取当前配置参数
 * @return 配置键值对
 */
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

/**
 * @brief 连接按钮点击
 */
void WsConfigPanel::onConnectClicked()
{
    // TODO: 创建WebSocketConnection，配置并打开
}

/**
 * @brief 初始化UI布局
 */
void WsConfigPanel::setupUi()
{
    auto* layout = new QVBoxLayout(this);

    // 连接类型
    m_typeCombo = new QComboBox(this);
    m_typeCombo->setObjectName("typeCombo");
    m_typeCombo->addItem("WS");
    m_typeCombo->addItem("WSS");

    // URL输入
    m_urlEdit = new QLineEdit(this);
    m_urlEdit->setObjectName("urlEdit");
    m_urlEdit->setPlaceholderText(tr("例如: 192.168.1.100:8080/ws"));

    // 子协议
    m_protocolEdit = new QLineEdit(this);
    m_protocolEdit->setObjectName("protocolEdit");
    m_protocolEdit->setPlaceholderText(tr("子协议(可选)"));

    // 连接按钮
    m_connectBtn = new QPushButton(tr("连接"), this);
    m_connectBtn->setObjectName("connectBtn");

    layout->addWidget(m_typeCombo);
    layout->addWidget(m_urlEdit);
    layout->addWidget(m_protocolEdit);
    layout->addWidget(m_connectBtn);
}

/**
 * @brief 初始化信号连接
 */
void WsConfigPanel::setupConnections()
{
    connect(m_connectBtn, &QPushButton::clicked,
            this, &WsConfigPanel::onConnectClicked);
}
