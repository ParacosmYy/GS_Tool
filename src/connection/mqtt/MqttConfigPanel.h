/**
 * @file MqttConfigPanel.h
 * @brief MQTT配置面板 — 提供MQTT服务器连接参数配置界面
 *
 * 职责: 收集MQTT连接参数(主机/端口/客户端ID/用户名/密码)，
 * 通过config()供上层获取。
 */
#ifndef MQTTCONFIGPANEL_H
#define MQTTCONFIGPANEL_H

#include <QWidget>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QVariantMap>

/**
 * @brief MQTT连接配置面板
 *
 * 提供服务器地址、端口、客户端ID、认证信息和连接按钮。
 * config()返回QVariantMap，包含host/port/clientId/username/password字段。
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
     * @return QVariantMap，包含host/port/clientId/username/password
     */
    QVariantMap config() const;

signals:
    /** @brief 用户点击连接按钮 */
    void connectRequested();

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

    /** @brief 连接按钮 */
    QPushButton* m_connectBtn;
};

#endif // MQTTCONFIGPANEL_H
