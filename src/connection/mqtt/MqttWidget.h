/**
 * @file MqttWidget.h
 * @brief MQTT客户端整体面板 — 集成配置/订阅/发布/状态指示/统计显示
 *
 * 职责: 组合MqttConfigPanel/MqttSubscriptionPanel/MqttTopicModel/MqttConnection，
 * 提供连接状态指示、自动重连、消息发布编辑器、统计面板。
 */
#ifndef MQTTWIDGET_H
#define MQTTWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QTextEdit>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QTimer>
#include <QGroupBox>
#include <QTreeView>
#include <QStatusBar>

#include "shared/AppConstants.h"

class MqttConnection;
class MqttConfigPanel;
class MqttSubscriptionPanel;
class MqttTopicModel;
class QCheckBox;

/**
 * @brief MQTT客户端整体面板
 *
 * 整合配置面板、订阅管理、消息发布、连接状态指示、统计信息。
 * 支持自动重连、QoS选择、LWT遗嘱配置、消息队列状态。
 */
class MqttWidget : public QWidget {
    Q_OBJECT

public:
    /**
     * @brief 构造MQTT整体面板
     * @param connection MQTT连接对象(外部拥有)
     * @param parent 父控件
     */
    explicit MqttWidget(MqttConnection* connection, QWidget* parent = nullptr);

    /** @brief 析构函数 */
    ~MqttWidget() override;

    /** @brief 获取内部MQTT连接对象 @return 连接指针 */
    MqttConnection* connection() const;

    /** @brief 获取主题模型 @return 模型指针 */
    MqttTopicModel* topicModel() const;

    /** @brief 获取累计自动重连次数 */
    quint64 totalReconnectAttempts() const { return m_totalReconnectAttempts; }

    /** @brief 获取累计发布操作次数(UI层面) */
    quint64 totalPublishOps() const { return m_totalPublishOps; }

    /** @brief 重置UI层统计计数器 */
    void resetWidgetStatistics();

signals:
    /** @brief 消息发布成功信号 @param topic 发布主题 @param payload 负载 */
    void messagePublished(const QString& topic, const QByteArray& payload);

private slots:
    /** @brief 连接/断开按钮处理 */
    void onConnectClicked();
    /** @brief 发布按钮处理 */
    void onPublishClicked();
    /** @brief 连接状态变更回调 */
    void onConnectionStateChanged(ConnectionState state);
    /** @brief 接收到MQTT消息回调 @param topic 消息主题 @param payload 消息负载 */
    void onMessageReceived(const QString& topic, const QByteArray& payload);
    /** @brief 自动重连定时器触发 */
    void onReconnectTimer();
    /** @brief 更新统计面板显示 */
    void updateStatistics();

private:
    /** @brief 初始化UI布局 */
    void setupUi();
    /** @brief 连接信号槽 */
    void connectSignals();
    /** @brief 启动自动重连计时器 */
    void startReconnect();
    /** @brief 停止自动重连计时器 */
    void stopReconnect();
    /** @brief 格式化字节大小为可读字符串 @param bytes 字节数 @return 格式化字符串 */
    QString formatBytes(quint64 bytes) const;

    // 核心组件
    MqttConnection* m_connection;            ///< MQTT连接(外部拥有)
    MqttConfigPanel* m_configPanel;          ///< 配置面板
    MqttSubscriptionPanel* m_subscriptionPanel; ///< 订阅管理面板
    MqttTopicModel* m_topicModel;            ///< 主题树模型

    // 连接状态指示
    QLabel* m_statusIndicator;               ///< 状态指示灯(圆形色块)
    QLabel* m_statusText;                    ///< 状态文本
    QTimer* m_reconnectTimer;                ///< 自动重连定时器
    bool m_autoReconnect = true;             ///< 自动重连开关
    int m_reconnectInterval = 5000;          ///< 重连间隔(ms)

    // 发布区域
    QLineEdit* m_pubTopicEdit;               ///< 发布主题输入框
    QComboBox* m_pubQosCombo;                ///< 发布QoS选择
    QTextEdit* m_pubPayloadEdit;             ///< 发布负载编辑器
    QPushButton* m_pubBtn;                   ///< 发布按钮
    QCheckBox* m_retainCheck;                ///< Retain标志

    // 统计面板
    QLabel* m_statPublished;                 ///< 发布统计标签
    QLabel* m_statReceived;                  ///< 接收统计标签
    QLabel* m_statQos0;                      ///< QoS0统计标签
    QLabel* m_statQos1;                      ///< QoS1统计标签
    QLabel* m_statQos2;                      ///< QoS2统计标签
    QLabel* m_statBytesSent;                 ///< 发送字节标签
    QLabel* m_statBytesReceived;             ///< 接收字节标签
    QLabel* m_statQueueSize;                 ///< 队列大小标签
    QLabel* m_statKeepAlive;                 ///< 心跳次数标签
    QLabel* m_statConnAttempts;              ///< 连接尝试标签
    QLabel* m_statLastConnect;               ///< 最后连接时间标签
    QTimer* m_statsRefreshTimer;             ///< 统计刷新定时器

    // 主题浏览器
    QTreeView* m_topicView;                  ///< 主题树视图

    // UI统计计数器
    quint64 m_totalReconnectAttempts = 0;    ///< 累计自动重连次数
    quint64 m_totalPublishOps = 0;           ///< 累计发布操作次数
};

#endif // MQTTWIDGET_H
