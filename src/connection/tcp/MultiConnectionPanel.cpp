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

/** @brief 构造多连接管理面板UI @param parent 父控件 */
MultiConnectionPanel::MultiConnectionPanel(QWidget* parent)
    : QWidget(parent)
{
    setObjectName("MultiConnectionPanel");
    setupUi();
    setupConnections();
}

/** @brief 设置后端TCP多连接管理器，连接connectionAdded/Removed信号 @param manager TCP多连接管理器 */
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

/** @brief 添加连接按钮点击，弹出对话框输入主机地址和端口号 */
void MultiConnectionPanel::onAddClicked()
{
    if (!m_manager) return;
    ++m_totalAddAttempts;

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

/** @brief 移除连接按钮点击，移除列表中当前选中的连接 */
void MultiConnectionPanel::onRemoveClicked()
{
    if (!m_connectionList || !m_connectionList->currentItem() || !m_manager) {
        return;
    }
    ++m_totalRemoveAttempts;
    int id = m_connectionList->currentItem()->data(Qt::UserRole).toInt();
    m_manager->removeConnection(id);
}

/** @brief 广播发送按钮点击，将输入框文本发送到所有已连接的TCP客户端 */
void MultiConnectionPanel::onSendAllClicked()
{
    if (!m_manager || !m_broadcastEdit) return;

    QString text = m_broadcastEdit->text();
    if (text.isEmpty()) return;

    QByteArray data = text.toUtf8();
    ++m_totalBroadcastsSent;
    int count = m_manager->sendToAll(data);
    if (m_statusLabel) {
        m_statusLabel->setText(tr("已发送到 %1 个连接").arg(count));
    }
}

/** @brief 新连接添加回调，在列表中创建对应条目 @param id 连接ID @param host 主机地址 @param port 端口号 */
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

/** @brief 连接移除回调，从列表中删除对应条目 @param id 连接ID */
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

// setupUi()/setupConnections() 已拆分至 MultiConnectionPanelUI.cpp

/** @brief 获取累计连接次数 @return 历史连接总数 */
quint64 MultiConnectionPanel::totalConnections() const
{
    return m_totalConnections;
}

/** @brief 获取累计断开次数 @return 历史断开总数 */
quint64 MultiConnectionPanel::totalDisconnections() const
{
    return m_totalDisconnections;
}

/** @brief 重置所有统计计数器(连接/断开/广播/添加/移除) */
void MultiConnectionPanel::resetStatistics()
{
    m_totalConnections = 0;
    m_totalDisconnections = 0;
    m_totalBroadcastsSent = 0;
    m_totalAddAttempts = 0;
    m_totalRemoveAttempts = 0;
}
