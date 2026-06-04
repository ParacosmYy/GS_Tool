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

    /** @brief 获取累计连接次数 */
    quint64 totalConnections() const;

    /** @brief 获取累计断开次数 */
    quint64 totalDisconnections() const;

    /** @brief 获取累计广播发送次数 */
    quint64 totalBroadcastsSent() const { return m_totalBroadcastsSent; }

    /** @brief 获取累计添加尝试次数(含用户取消) */
    quint64 totalAddAttempts() const { return m_totalAddAttempts; }

    /** @brief 获取累计移除尝试次数 */
    quint64 totalRemoveAttempts() const { return m_totalRemoveAttempts; }

    /** @brief 重置所有统计计数器 */
    void resetStatistics();

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

    // ---- 统计计数器 ----
    quint64 m_totalConnections = 0;                   ///< 累计连接次数
    quint64 m_totalDisconnections = 0;                ///< 累计断开次数
    quint64 m_totalBroadcastsSent = 0;                ///< 累计广播发送次数
    quint64 m_totalAddAttempts = 0;                   ///< 累计添加尝试次数
    quint64 m_totalRemoveAttempts = 0;                ///< 累计移除尝试次数
};

#endif // MULTICONNECTIONPANEL_H
