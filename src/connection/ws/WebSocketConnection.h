/**
 * @file WebSocketConnection.h
 * @brief WebSocket客户端连接 - 支持WS/WSS协议的WebSocket通信
 *
 * 职责:
 *   1. 提供WebSocket客户端连接能力
 *   2. 支持文本帧和二进制帧收发
 *   3. 支持心跳检测(ping/pong)
 *   4. 复用IConnection抽象接口
 *
 * 协作关系:
 *   - ConnectionFactory: 通过工厂创建实例
 *   - WsConfigPanel: WebSocket参数配置UI
 */

#ifndef WEBSOCKETCONNECTION_H
#define WEBSOCKETCONNECTION_H

#include "connection/interface/IConnection.h"
#include <QTcpSocket>
#include <QTimer>

/**
 * @brief WebSocket客户端连接实现
 *
 * 支持ws://和wss://协议，可收发文本和二进制消息。
 * 内置心跳定时器，定期发送ping帧保持连接活跃。
 */
class WebSocketConnection : public IConnection {
    Q_OBJECT

public:
    /** @brief 构造函数
     * @param parent 父对象
     */
    explicit WebSocketConnection(QObject* parent = nullptr);

    /** @brief 析构，关闭连接 */
    ~WebSocketConnection() override;

    // ---- IConnection接口实现 ----
    ConnectionType type() const override;
    QString name() const override;
    ConnectionState state() const override;
    bool open() override;
    void close() override;
    qint64 write(const QByteArray& data) override;
    void configure(const QVariantMap& params) override;

    // ---- WebSocket特有接口 ----

    /**
     * @brief 连接到指定URL
     * @param url WebSocket地址(如ws://host:port/path)
     * @return true=连接已发起
     */
    bool connectToUrl(const QString& url);

    /**
     * @brief 发送文本消息
     * @param message 文本内容
     * @return 发送字节数
     */
    qint64 sendTextMessage(const QString& message);

    /**
     * @brief 发送二进制消息
     * @param data 二进制数据
     * @return 发送字节数
     */
    qint64 sendBinaryMessage(const QByteArray& data);

    /**
     * @brief 发送ping帧
     * @param payload ping载荷数据
     * @return true=发送成功
     */
    bool ping(const QByteArray& payload = QByteArray());

signals:
    /** @brief 收到文本消息时发出
     * @param message 文本内容
     */
    void textMessageReceived(const QString& message);

    /** @brief 收到二进制消息时发出
     * @param data 二进制数据
     */
    void binaryMessageReceived(const QByteArray& data);

    /** @brief 收到pong响应时发出
     * @param payload pong载荷数据
     */
    void pongReceived(const QByteArray& payload);

private slots:
    /** @brief WebSocket连接成功回调 */
    void onConnected();

    /** @brief WebSocket断开回调 */
    void onDisconnected();

    /** @brief 收到文本消息回调 */
    void onTextMessageReceived(const QString& message);

    /** @brief 收到二进制消息回调 */
    void onBinaryMessageReceived(const QByteArray& data);

    /** @brief 心跳定时器触发 */
    void onPingTimeout();

private:
    /** @brief 更新连接状态 */
    void updateState(ConnectionState newState);

    // ---- 配置参数 ----
    QString m_url;                                  ///< WebSocket服务器地址
    QString m_protocol;                             ///< 子协议名称
    ConnectionState m_state = ConnectionState::Disconnected; ///< 当前状态

    // ---- 网络资源 ----
    QTcpSocket* m_socket = nullptr;                  ///< TCP底层socket (WebSockets骨架，待引入QtWebSockets模块)
    QTimer* m_pingTimer = nullptr;                   ///< 心跳定时器
};

#endif // WEBSOCKETCONNECTION_H
