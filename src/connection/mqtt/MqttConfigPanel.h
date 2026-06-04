/**
 * @file MqttConfigPanel.h
 * @brief MQTT配置面板 — 提供MQTT服务器连接参数配置界面
 *
 * 职责: 收集MQTT连接参数(主机/端口/客户端ID/用户名/密码/KeepAlive/Clean Session)，
 * 通过config()供上层获取。支持连接/断开切换和状态显示。
 */
#ifndef MQTTCONFIGPANEL_H
#define MQTTCONFIGPANEL_H

#include <QWidget>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QLabel>
#include <QCheckBox>
#include <QSettings>
#include <QVariantMap>

/**
 * @brief MQTT连接配置面板
 *
 * 提供服务器地址、端口、客户端ID、认证信息、KeepAlive、Clean Session和连接按钮。
 * config()返回QVariantMap，包含host/port/clientId/username/password/keepAlive/cleanSession字段。
 */
class MqttConfigPanel : public QWidget {
    Q_OBJECT

public:
    /**
     * @brief 构造MQTT配置面板
     * @param parent 父控件
     */
    explicit MqttConfigPanel(QWidget* parent = nullptr);

    /**
     * @brief 获取当前配置参数
     * @return QVariantMap，包含host/port/clientId/username/password/keepAlive/cleanSession
     */
    QVariantMap config() const;

    /** @brief 获取累计连接尝试次数 @return 连接尝试总次数 */
    quint64 totalConnectAttempts() const { return m_totalConnectAttempts; }

    /** @brief 获取累计配置变更次数 @return 配置变更总次数 */
    quint64 totalConfigChanges() const { return m_totalConfigChanges; }

    /** @brief 重置所有统计计数器 */
    void resetStatistics();

    /**
     * @brief 设置连接状态(更新按钮文本和状态标签)
     * @param connected true=已连接
     */
    void setConnected(bool connected);

    /**
     * @brief 保存配置到QSettings
     * @param settings QSettings对象
     */
    void saveSettings(QSettings& settings) const;

    /**
     * @brief 从QSettings加载配置
     * @param settings QSettings对象
     */
    void loadSettings(QSettings& settings);

signals:
    /** @brief 用户点击连接按钮 */
    void connectRequested();
    /** @brief 用户点击断开按钮 */
    void disconnectRequested();

private:
    /** @brief 服务器地址输入框 */
    QLineEdit* m_hostEdit;

    /** @brief 端口微调框 */
    QSpinBox* m_portSpin;

    /** @brief 客户端ID输入框 */
    QLineEdit* m_clientIdEdit;

    /** @brief 用户名输入框 */
    QLineEdit* m_usernameEdit;

    /** @brief 密码输入框 */
    QLineEdit* m_passwordEdit;

    /** @brief KeepAlive间隔微调框(秒) */
    QSpinBox* m_keepAliveSpin;

    /** @brief Clean Session复选框 */
    QCheckBox* m_cleanSessionCheck;

    /** @brief 连接/断开按钮 */
    QPushButton* m_connectBtn;

    /** @brief 状态标签 */
    QLabel* m_statusLabel;

    /** @brief 当前是否已连接 */
    bool m_connected = false;

    // ---- 统计计数器 ----
    quint64 m_totalConnectAttempts = 0;  ///< 累计连接尝试次数
    quint64 m_totalConfigChanges = 0;    ///< 累计配置变更次数
};

#endif // MQTTCONFIGPANEL_H
