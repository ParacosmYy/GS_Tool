/**
 * @file WsConfigPanel.h
 * @brief WebSocket配置面板 - 配置WebSocket连接参数的UI
 *
 * 职责:
 *   1. 提供URL、子协议、连接类型(WS/WSS)配置
 *   2. 提供连接/断开操作按钮
 *   3. 显示连接状态
 */

#ifndef WSCONFIGPANEL_H
#define WSCONFIGPANEL_H

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QSettings>
#include <QVBoxLayout>
#include <QVariant>

/**
 * @brief WebSocket配置面板UI
 *
 * 提供WebSocket连接所需的URL、子协议和安全模式配置。
 */
class WsConfigPanel : public QWidget {
    Q_OBJECT

public:
    /** @brief 构造函数
     * @param parent 父控件
     */
    explicit WsConfigPanel(QWidget* parent = nullptr);

    /**
     * @brief 获取当前配置参数
     * @return 配置键值对(QVariantMap)
     */
    QVariantMap config() const;

    /** @brief 获取累计连接尝试次数 */
    quint64 totalConnectAttempts() const { return m_totalConnectAttempts; }

    /** @brief 获取累计断开连接次数 */
    quint64 totalDisconnections() const { return m_totalDisconnections; }

    /** @brief 重置所有统计计数器 */
    void resetStatistics();

    /**
     * @brief 设置连接状态(更新按钮文本和状态标签)
     * @param connected true=已连接
     */
    void setConnected(bool connected);

    /**
     * @brief 保存配置到QSettings
     * @param settings QSettings对象（调用方管理生命周期）
     */
    void saveSettings(QSettings& settings) const;

    /**
     * @brief 从QSettings加载配置
     * @param settings QSettings对象
     */
    void loadSettings(QSettings& settings);

signals:
    /** @brief 用户请求连接 */
    void connectRequested(const QVariantMap& config);

    /** @brief 用户请求断开 */
    void disconnectRequested();

private slots:
    /** @brief 连接按钮点击 */
    void onConnectClicked();

private:
    /** @brief 初始化UI布局 */
    void setupUi();

    /** @brief 初始化信号连接 */
    void setupConnections();

    // ---- UI控件 ----
    QLineEdit* m_urlEdit = nullptr;      ///< URL输入框
    QLineEdit* m_protocolEdit = nullptr; ///< 子协议输入框
    QComboBox* m_typeCombo = nullptr;    ///< 连接类型(WS/WSS)
    QPushButton* m_connectBtn = nullptr; ///< 连接按钮
    QLabel* m_statusLabel = nullptr;     ///< 状态标签

    // ---- 状态 ----
    bool m_connected = false;            ///< 当前连接状态

    // ---- 统计计数器 ----
    quint64 m_totalConnectAttempts = 0;  ///< 累计连接尝试次数
    quint64 m_totalDisconnections = 0;   ///< 累计断开连接次数
};

#endif // WSCONFIGPANEL_H
