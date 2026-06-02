/**
 * @file MqttConnection.h
 * @brief MQTT客户端连接实现 — 基于QTcpSocket实现MQTT v3.1.1线协议
 *
 * 职责: MQTT连接管理、消息发布/订阅、KeepAlive心跳，
 * 通过IConnection统一接口供上层使用。
 */
#ifndef MQTTCONNECTION_H
#define MQTTCONNECTION_H

#include "connection/interface/IConnection.h"
#include <QTcpSocket>
#include <QTimer>

/**
 * @brief MQTT客户端连接实现
 *
 * 封装MQTT v3.1.1协议通信，基于QTcpSocket实现原始线协议。
 * 支持CONNECT/PUBLISH/SUBSCRIBE/UNSUBSCRIBE/PINGREQ等基本报文类型。
 */
class MqttConnection : public IConnection {
    Q_OBJECT

public:
    /**
     * @brief 构造MQTT连接
     * @param parent 父对象
     */
    explicit MqttConnection(QObject* parent = nullptr);

    /** @brief 析构函数，自动断开连接 */
    ~MqttConnection() override;

    // ---- IConnection 接口实现 ----

    /** @brief 返回连接类型 */
    ConnectionType type() const override;

    /** @brief 返回MQTT服务器地址 */
    QString name() const override;

    /** @brief 返回当前连接状态 */
    ConnectionState state() const override;

    /** @brief 打开MQTT连接 */
    bool open() override;

    /** @brief 关闭MQTT连接 */
    void close() override;

    /**
     * @brief 写入(发布)数据到默认主题
     * @param data 待发布的数据
     * @return 实际写入字节数，-1表示失败
     */
    qint64 write(const QByteArray& data) override;

    /**
     * @brief 通过参数映射配置MQTT连接
     * @param params 支持的key: host, port, clientId, username, password, keepAlive, cleanSession
     */
    void configure(const QVariantMap& params) override;

    // ---- MQTT专用接口 ----

    /**
     * @brief 连接到MQTT服务器
     * @param host 服务器地址
     * @param port 端口号
     */
    void connectToHost(const QString& host, int port);

    /** @brief 断开MQTT服务器连接 */
    void disconnectFromHost();

    /**
     * @brief 发布消息到指定主题
     * @param topic 目标主题
     * @param payload 消息负载
     * @param qos 服务质量等级(0/1/2)
     * @return true=发布成功
     */
    bool publish(const QString& topic, const QByteArray& payload, int qos = 0);

    /**
     * @brief 订阅主题
     * @param topic 订阅主题(支持通配符#和+)
     * @param qos 服务质量等级
     * @return true=订阅请求已发送
     */
    bool subscribe(const QString& topic, int qos = 0);

    /**
     * @brief 取消订阅主题
     * @param topic 要取消的主题
     */
    void unsubscribe(const QString& topic);

signals:
    /**
     * @brief 收到MQTT消息
     * @param topic 消息主题
     * @param payload 消息负载
     */
    void messageReceived(const QString& topic, const QByteArray& payload);

    /** @brief MQTT连接已建立 */
    void connected();

    /** @brief MQTT连接已断开 */
    void disconnected();

private slots:
    /** @brief TCP socket数据到达处理 */
    void onSocketReadyRead();

    /** @brief TCP连接建立后发送MQTT CONNECT */
    void onSocketConnected();

    /** @brief TCP连接断开处理 */
    void onSocketDisconnected();

    /** @brief 发送PINGREQ保活 */
    void onKeepAlive();

private:
    /**
     * @brief 构建MQTT固定头
     * @param packetType 报文类型(1=CONNECT, 3=PUBLISH, 8=SUBSCRIBE, ...)
     * @param payload 变长头+负载
     * @return 完整MQTT报文
     */
    QByteArray buildMqttPacket(quint8 packetType, const QByteArray& payload);

    /** @brief 编码剩余长度字段 */
    QByteArray encodeRemainingLength(int length);

    /** @brief 解析收到的MQTT报文并分发 */
    void parseIncomingPacket();

    /** @brief 发送MQTT CONNECT报文 */
    void sendConnect();

    /** @brief 处理CONNACK报文 */
    void handleConnack(const QByteArray& data);

    /** @brief 处理PUBLISH报文(服务器推送) */
    void handlePublish(const QByteArray& data, quint8 flags);

    /** @brief 处理SUBACK报文 */
    void handleSuback(const QByteArray& data);

    /** @brief 生成自动客户端ID */
    QString generateClientId();

    /** @brief MQTT服务器地址 */
    QString m_host;

    /** @brief MQTT服务器端口，默认1883 */
    int m_port = 1883;

    /** @brief 客户端ID */
    QString m_clientId;

    /** @brief 认证用户名 */
    QString m_username;

    /** @brief 认证密码 */
    QString m_password;

    /** @brief KeepAlive间隔(秒)，默认60 */
    int m_keepAliveInterval = 60;

    /** @brief Clean Session标志 */
    bool m_cleanSession = true;

    /** @brief 报文标识符计数器(用于SUB/UNSUB) */
    quint16 m_packetId = 0;

    /** @brief TCP socket */
    QTcpSocket* m_socket;

    /** @brief KeepAlive心跳定时器 */
    QTimer* m_keepAlive;

    /** @brief 当前连接状态 */
    ConnectionState m_state = ConnectionState::Disconnected;

    /** @brief 接收缓冲区 */
    QByteArray m_rxBuffer;

    /** @brief 期望的剩余长度(解析中间状态) */
    int m_expectedLength = -1;
};

#endif // MQTTCONNECTION_H
