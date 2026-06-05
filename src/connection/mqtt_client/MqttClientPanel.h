/**
 * @file MqttClientPanel.h
 * @brief MQTT客户端面板 — 提供连接配置、主题订阅、消息发布、消息日志界面
 *
 * 职责: 组合MqttClientEngine，提供完整的MQTT调试交互面板。
 * 包含连接参数表单、订阅管理、发布表单、接收消息日志四个区域。
 */

#ifndef MQTTCLIENTPANEL_H
#define MQTTCLIENTPANEL_H

#include "connection/mqtt_client/MqttClientEngine.h"

#include <QWidget>
#include <QLineEdit>
#include <QSpinBox>
#include <QComboBox>
#include <QPushButton>
#include <QTextEdit>
#include <QListWidget>
#include <QLabel>
#include <QCheckBox>
#include <QGroupBox>
#include <QSplitter>
#include <QTimer>

/**
 * @brief MQTT客户端交互面板
 *
 * 顶部: 连接参数（Broker/Port/Username/Password/TLS/KeepAlive）+ 连接/断开按钮
 * 左侧: 订阅主题输入 + 已订阅主题列表 + 退订按钮
 * 右侧上: 发布表单（Topic/Payload/QoS/Retain）+ 发布按钮
 * 右侧下: 接收消息日志（只读文本区）+ 清空按钮
 */
class MqttClientPanel : public QWidget {
    Q_OBJECT

public:
    /** @brief 构造MQTT客户端面板 @param parent 父控件 */
    explicit MqttClientPanel(QWidget* parent = nullptr);

    /** @brief 析构 */
    ~MqttClientPanel() override = default;

    /** @brief 获取内部引擎指针 @return 引擎指针（非空） */
    MqttClientEngine* engine() const;

    /** @brief 获取累计发布按钮点击次数 @return 点击计数 */
    quint64 totalPublishClicks() const { return m_totalPublishClicks; }
    /** @brief 获取累计订阅操作次数 @return 订阅计数 */
    quint64 totalSubscribeOps() const { return m_totalSubscribeOps; }
    /** @brief 获取累计连接按钮点击次数 @return 连接计数 */
    quint64 totalConnectClicks() const { return m_totalConnectClicks; }
    /** @brief 重置面板统计计数器 */
    void resetStats();

private slots:
    void onConnectClicked();
    void onDisconnectClicked();
    void onSubscribeClicked();
    void onUnsubscribeClicked();
    void onPublishClicked();
    void onClearLogClicked();
    void onEngineConnected();
    void onEngineDisconnected();
    void onEngineMessageReceived(const MqttMessage& msg);
    void onEngineError(const QString& error);

private:
    void setupUI();
    QGroupBox* createConnectionGroup();
    QGroupBox* createSubscribeGroup();
    QGroupBox* createPublishGroup();
    QGroupBox* createLogGroup();
    void updateConnectionState(bool connected);

    MqttClientEngine* m_engine;

    /* 连接表单 */
    QLineEdit*  m_brokerEdit;
    QSpinBox*   m_portSpin;
    QLineEdit*  m_clientIdEdit;
    QLineEdit*  m_usernameEdit;
    QLineEdit*  m_passwordEdit;
    QCheckBox*  m_tlsCheck;
    QSpinBox*   m_keepAliveSpin;
    QPushButton* m_connectBtn;
    QPushButton* m_disconnectBtn;
    QLabel*     m_statusLabel;

    /* 订阅区域 */
    QLineEdit*  m_subTopicEdit;
    QComboBox*  m_subQosCombo;
    QPushButton* m_subBtn;
    QListWidget* m_subListWidget;
    QPushButton* m_unsubBtn;

    /* 发布区域 */
    QLineEdit*  m_pubTopicEdit;
    QTextEdit*  m_pubPayloadEdit;
    QComboBox*  m_pubQosCombo;
    QCheckBox*  m_pubRetainCheck;
    QPushButton* m_pubBtn;

    /* 消息日志 */
    QTextEdit*  m_logEdit;
    QPushButton* m_clearLogBtn;

    /* 统计 */
    quint64 m_totalPublishClicks;
    quint64 m_totalSubscribeOps;
    quint64 m_totalConnectClicks;
};

#endif // MQTTCLIENTPANEL_H
