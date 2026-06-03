/**
 * @file MultiConnectionPanel.cpp
 * @brief 多连接管理面板实现
 */

#include "connection/tcp/MultiConnectionPanel.h"
#include "connection/tcp/TcpMultiConnectionManager.h"
#include <QInputDialog>
#include <QDialog>
#include <QFormLayout>
#include <QDialogButtonBox>
#include "core/widgets/EdDialog.h"
#include <QSpinBox>

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
 * @brief 添加连接按钮点击 - 弹出对话框输入主机和端口
 */
void MultiConnectionPanel::onAddClicked()
{
    if (!m_manager) return;

    /// 创建输入对话框
    QDialog dialog(this);
    dialog.setWindowTitle(tr("添加TCP连接"));
    dialog.setObjectName("addConnectionDialog");

    auto* layout = new QFormLayout(&dialog);

    auto* hostEdit = new QLineEdit("127.0.0.1", &dialog);
    hostEdit->setObjectName("tcpMultiHostEdit");
    hostEdit->setPlaceholderText(tr("输入主机地址"));

    auto* portSpin = new QSpinBox(&dialog);
    portSpin->setObjectName("tcpMultiPortSpin");
    portSpin->setRange(1, 65535);
    portSpin->setValue(8080);

    auto* buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);

    layout->addRow(tr("主机地址:"), hostEdit);
    layout->addRow(tr("端口号:"), portSpin);
    layout->addRow(buttons);

    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        QString host = hostEdit->text().trimmed();
        int port = portSpin->value();
        if (host.isEmpty()) {
            EdDialog::warning(this, tr("输入错误"), tr("主机地址不能为空"));
            return;
        }
        m_manager->addConnection(host, port);
    }
}

/**
 * @brief 移除连接按钮点击
 */
void MultiConnectionPanel::onRemoveClicked()
{
    if (!m_connectionList || !m_connectionList->currentItem() || !m_manager) {
        return;
    }
    int id = m_connectionList->currentItem()->data(Qt::UserRole).toInt();
    m_manager->removeConnection(id);
}

/**
 * @brief 广播发送按钮点击
 */
void MultiConnectionPanel::onSendAllClicked()
{
    if (!m_manager || !m_broadcastEdit) return;

    QString text = m_broadcastEdit->text();
    if (text.isEmpty()) return;

    QByteArray data = text.toUtf8();
    int count = m_manager->sendToAll(data);
    if (m_statusLabel) {
        m_statusLabel->setText(tr("已发送到 %1 个连接").arg(count));
    }
}

/**
 * @brief 新连接添加回调
 * @param id 连接ID
 * @param host 主机地址
 * @param port 端口号
 */
void MultiConnectionPanel::onConnectionAdded(int id, const QString& host, int port)
{
    if (!m_connectionList) return;
    auto* item = new QListWidgetItem(
        tr("#%1 %2:%3").arg(id).arg(host).arg(port), m_connectionList);
    item->setData(Qt::UserRole, id);

    if (m_statusLabel) {
        m_statusLabel->setText(tr("连接数: %1").arg(m_connectionList->count()));
    }
    ++m_totalConnections;
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
    if (m_statusLabel) {
        m_statusLabel->setText(tr("连接数: %1").arg(m_connectionList->count()));
    }
    ++m_totalDisconnections;
}

/**
 * @brief 初始化UI布局
 */
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

/**
 * @brief 初始化信号连接
 */
void MultiConnectionPanel::setupConnections()
{
    connect(m_addBtn, &QPushButton::clicked,
            this, &MultiConnectionPanel::onAddClicked);
    connect(m_removeBtn, &QPushButton::clicked,
            this, &MultiConnectionPanel::onRemoveClicked);
    connect(m_sendAllBtn, &QPushButton::clicked,
            this, &MultiConnectionPanel::onSendAllClicked);
}

/** @brief 获取累计连接次数 */
quint64 MultiConnectionPanel::totalConnections() const
{
    return m_totalConnections;
}

/** @brief 获取累计断开次数 */
quint64 MultiConnectionPanel::totalDisconnections() const
{
    return m_totalDisconnections;
}

/** @brief 重置所有统计计数器 */
void MultiConnectionPanel::resetStatistics()
{
    m_totalConnections = 0;
    m_totalDisconnections = 0;
}
