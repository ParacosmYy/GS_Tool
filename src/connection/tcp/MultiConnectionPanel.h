/**
 * @file MultiConnectionPanel.h
 * @brief 多连接管理面板 - 管理多个TCP连接的UI界面
 *
 * 职责:
 *   1. 显示已添加的TCP连接列表
 *   2. 提供添加/移除连接的操作按钮
 *   3. 显示各连接的状态信息
 *   4. 支持广播消息发送
 *
 * 协作关系:
 *   - TcpMultiConnectionManager: 后端多连接管理器
 */

#ifndef MULTICONNECTIONPANEL_H
#define MULTICONNECTIONPANEL_H

#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>

class TcpMultiConnectionManager;

/**
 * @brief 多连接管理面板UI
 *
 * 提供TCP多连接的可视化管理，包括添加新连接、
 * 移除连接、查看连接状态和广播消息发送。
 */
class MultiConnectionPanel : public QWidget {
    Q_OBJECT

public:
    /** @brief 构造函数
     * @param parent 父控件
     */
    explicit MultiConnectionPanel(QWidget* parent = nullptr);

    /**
     * @brief 设置后端管理器
     * @param manager TCP多连接管理器实例
     */
    void setManager(TcpMultiConnectionManager* manager);

private slots:
    /** @brief 添加连接按钮点击 */
    void onAddClicked();

    /** @brief 移除连接按钮点击 */
    void onRemoveClicked();

    /** @brief 广播发送按钮点击 */
    void onSendAllClicked();

    /** @brief 新连接添加回调 */
    void onConnectionAdded(int id, const QString& host, int port);

    /** @brief 连接移除回调 */
    void onConnectionRemoved(int id);

private:
    /** @brief 初始化UI布局 */
    void setupUi();

    /** @brief 初始化信号连接 */
    void setupConnections();

    TcpMultiConnectionManager* m_manager = nullptr;  ///< 后端管理器

    // ---- UI控件 ----
    QListWidget* m_connectionList = nullptr;          ///< 连接列表控件
    QPushButton* m_addBtn = nullptr;                  ///< 添加连接按钮
    QPushButton* m_removeBtn = nullptr;               ///< 移除连接按钮
    QLineEdit* m_broadcastEdit = nullptr;             ///< 广播消息输入框
    QPushButton* m_sendAllBtn = nullptr;              ///< 广播发送按钮
    QLabel* m_statusLabel = nullptr;                  ///< 状态信息标签
};

#endif // MULTICONNECTIONPANEL_H
