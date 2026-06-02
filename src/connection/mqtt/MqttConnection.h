/**
 * @file MqttConnection.h
 * @brief MQTT客户端连接实现 — 适配器模式，封装MQTT协议到IConnection接口
 *
 * 职责: MQTT连接管理、消息发布/订阅、KeepAlive心跳，
 * 通过IConnection统一接口供上层使用。
 */
#ifndef MQTTCONNECTION_H
#define MQTTCONNECTION_H

#include "connection/interface/IConnection.h"
#include <QTimer>

/**
 * @brief MQTT客户端连接实现
 *
 * 封装MQTT协议通信，实现IConnection统一接口。
 * 额外提供主题订阅/取消订阅和消息发布接口。
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
     * @param params 支持的key: host, port, clientId, username, password
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

private:
    /** @brief MQTT服务器地址 */
    QString m_host;

    /** @brief MQTT服务器端口，默认1883 */
    int m_port = 1883;

    /** @brief KeepAlive心跳定时器 */
    QTimer* m_keepAlive;

    /** @brief 当前连接状态 */
    ConnectionState m_state = ConnectionState::Disconnected;
};

#endif // MQTTCONNECTION_H
