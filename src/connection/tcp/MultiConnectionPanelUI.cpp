/**
 * @file MultiConnectionPanelUI.cpp
 * @brief 多连接管理面板 — UI布局构建与信号连接
 *
 * 从 MultiConnectionPanel.cpp 拆分而来，集中管理面板的
 * UI初始化(setupUi)和信号连接(setupConnections)。
 *
 * @see MultiConnectionPanel.cpp — 业务逻辑(添加/移除/广播/统计)
 */

#include "connection/tcp/MultiConnectionPanel.h"

/** @brief 初始化UI布局: 连接列表/添加移除按钮/广播输入/状态标签 */
void MultiConnectionPanel::setupUi()
{
    auto* layout = new QVBoxLayout(this);

    /// 连接列表
    m_connectionList = new QListWidget(this);
    m_connectionList->setObjectName("tcpMultiConnectionList");

    /// 按钮行：添加 / 移除
    auto* btnLayout = new QHBoxLayout();
    m_addBtn = new QPushButton(tr("添加连接"), this);
    m_addBtn->setObjectName("tcpMultiAddBtn");

    m_removeBtn = new QPushButton(tr("移除连接"), this);
    m_removeBtn->setObjectName("tcpMultiRemoveBtn");

    btnLayout->addWidget(m_addBtn);
    btnLayout->addWidget(m_removeBtn);

    /// 广播消息输入和发送
    auto* sendLayout = new QHBoxLayout();
    m_broadcastEdit = new QLineEdit(this);
    m_broadcastEdit->setObjectName("tcpMultiBroadcastEdit");
    m_broadcastEdit->setPlaceholderText(tr("输入广播消息..."));

    m_sendAllBtn = new QPushButton(tr("发送全部"), this);
    m_sendAllBtn->setObjectName("tcpMultiSendAllBtn");

    sendLayout->addWidget(m_broadcastEdit);
    sendLayout->addWidget(m_sendAllBtn);

    /// 状态标签
    m_statusLabel = new QLabel(tr("连接数: 0"), this);
    m_statusLabel->setObjectName("tcpMultiStatusLabel");

    layout->addWidget(m_connectionList);
    layout->addLayout(btnLayout);
    layout->addLayout(sendLayout);
    layout->addWidget(m_statusLabel);
}

/** @brief 初始化信号连接: 添加/移除/广播发送按钮 */
void MultiConnectionPanel::setupConnections()
{
    connect(m_addBtn, &QPushButton::clicked,
            this, &MultiConnectionPanel::onAddClicked);
    connect(m_removeBtn, &QPushButton::clicked,
            this, &MultiConnectionPanel::onRemoveClicked);
    connect(m_sendAllBtn, &QPushButton::clicked,
            this, &MultiConnectionPanel::onSendAllClicked);
}
