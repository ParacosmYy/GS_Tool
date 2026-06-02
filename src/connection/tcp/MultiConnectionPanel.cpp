/**
 * @file MultiConnectionPanel.cpp
 * @brief 多连接管理面板实现 - 骨架
 */

#include "connection/tcp/MultiConnectionPanel.h"
#include "connection/tcp/TcpMultiConnectionManager.h"

/**
 * @brief 构造函数 - 初始化UI
 * @param parent 父控件
 */
MultiConnectionPanel::MultiConnectionPanel(QWidget* parent)
    : QWidget(parent)
{
    setObjectName("MultiConnectionPanel");
    setupUi();
    setupConnections();
}

/**
 * @brief 设置后端管理器
 * @param manager TCP多连接管理器
 */
void MultiConnectionPanel::setManager(TcpMultiConnectionManager* manager)
{
    if (m_manager) {
        disconnect(m_manager, nullptr, this, nullptr);
    }
    m_manager = manager;
    if (m_manager) {
        connect(m_manager, &TcpMultiConnectionManager::connectionAdded,
                this, &MultiConnectionPanel::onConnectionAdded);
        connect(m_manager, &TcpMultiConnectionManager::connectionRemoved,
                this, &MultiConnectionPanel::onConnectionRemoved);
    }
}

/**
 * @brief 添加连接按钮点击
 */
void MultiConnectionPanel::onAddClicked()
{
    // TODO: 弹出对话框输入主机和端口，调用m_manager->addConnection()
}

/**
 * @brief 移除连接按钮点击
 */
void MultiConnectionPanel::onRemoveClicked()
{
    // TODO: 获取当前选中项，调用m_manager->removeConnection()
    if (!m_connectionList || !m_connectionList->currentItem()) {
        return;
    }
    int id = m_connectionList->currentItem()->data(Qt::UserRole).toInt();
    if (m_manager) {
        m_manager->removeConnection(id);
    }
}

/**
 * @brief 新连接添加回调
 * @param id 连接ID
 */
void MultiConnectionPanel::onConnectionAdded(int id)
{
    if (!m_connectionList) return;
    auto* item = new QListWidgetItem(tr("连接 #%1").arg(id), m_connectionList);
    item->setData(Qt::UserRole, id);
}

/**
 * @brief 连接移除回调
 * @param id 连接ID
 */
void MultiConnectionPanel::onConnectionRemoved(int id)
{
    if (!m_connectionList) return;
    for (int i = 0; i < m_connectionList->count(); ++i) {
        if (m_connectionList->item(i)->data(Qt::UserRole).toInt() == id) {
            delete m_connectionList->takeItem(i);
            break;
        }
    }
}

/**
 * @brief 初始化UI布局
 */
void MultiConnectionPanel::setupUi()
{
    auto* layout = new QVBoxLayout(this);

    m_connectionList = new QListWidget(this);
    m_connectionList->setObjectName("connectionList");

    m_addBtn = new QPushButton(tr("添加连接"), this);
    m_addBtn->setObjectName("addBtn");

    m_removeBtn = new QPushButton(tr("移除连接"), this);
    m_removeBtn->setObjectName("removeBtn");

    auto* btnLayout = new QHBoxLayout();
    btnLayout->addWidget(m_addBtn);
    btnLayout->addWidget(m_removeBtn);

    layout->addWidget(m_connectionList);
    layout->addLayout(btnLayout);
}

/**
 * @brief 初始化信号连接
 */
void MultiConnectionPanel::setupConnections()
{
    connect(m_addBtn, &QPushButton::clicked,
            this, &MultiConnectionPanel::onAddClicked);
    connect(m_removeBtn, &QPushButton::clicked,
            this, &MultiConnectionPanel::onRemoveClicked);
}
