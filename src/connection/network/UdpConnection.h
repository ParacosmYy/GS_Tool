/**
 * @file UdpConnection.h
 * @brief UDP连接实现 - 支持单播/广播的UDP数据报收发
 *
 * 职责: 单播/广播UDP数据报收发，复用IConnection抽象接口
 * 设计模式: 策略模式 — UdpConnection是IConnection的一个具体策略实现
 * 协作: ConnectionFactory(创建) / ConnectionController(生命周期管理)
 * 注意: UDP是无连接协议，"Connected"状态表示socket已绑定本地端口
 */
#ifndef UDPCONNECTION_H
#define UDPCONNECTION_H

#include "connection/interface/IConnection.h"
#include <QUdpSocket>
#include <QHostAddress>

/** @brief UDP连接实现 - 支持单播/广播数据报收发 */
class UdpConnection : public IConnection {
    Q_OBJECT

public:
    explicit UdpConnection(QObject* parent = nullptr); ///< 构造UDP连接
    ~UdpConnection() override;                         ///< 析构，关闭socket并释放资源
    // ---- IConnection接口 ----
    ConnectionType type() const override;              ///< 返回连接类型(UDP)
    QString name() const override;                     ///< 返回连接显示名称
    ConnectionState state() const override;            ///< 返回当前连接状态
    /** @brief 打开UDP连接(绑定本地端口) @return true=绑定成功 */
    bool open() override;
    void close() override;                             ///< 关闭UDP连接
    /** @brief 发送UDP数据报 @param data 待发送数据 @return 实际发送字节数，-1表示失败 */
    qint64 write(const QByteArray& data) override;
    /** @brief 配置UDP连接参数(localPort/remoteHost/remotePort/broadcast) */
    void configure(const QVariantMap& params) override;
    // ---- 统计接口 ----
    quint64 totalDatagramsSent() const;     ///< 获取已发送数据报总数
    quint64 totalDatagramsReceived() const; ///< 获取已接收数据报总数
    quint64 totalBytesSent() const;         ///< 获取已发送字节总数
    quint64 totalBytesReceived() const;     ///< 获取已接收字节总数
    quint64 errorCount() const;             ///< 获取错误计数
    quint64 totalErrors() const { return m_errorCount; } ///< 获取累计错误次数(errorCount别名)
    quint64 totalDatagramErrors() const { return m_totalDatagramErrors; } ///< 获取数据报错误次数
    quint64 totalBroadcastsSent() const { return m_totalBroadcastsSent; } ///< 获取广播发送次数
    quint64 totalBroadcasts() const { return m_totalBroadcastsSent; } ///< totalBroadcastsSent别名
    quint64 totalSocketErrors() const { return m_totalSocketErrors; } ///< 获取Socket错误次数
    quint64 totalMulticastJoins() const { return m_totalMulticastJoins; } ///< 获取多播组加入次数
    quint64 totalMulticastLeaves() const { return m_totalMulticastLeaves; } ///< 获取多播组离开次数
    quint64 totalOpenAttempts() const { return m_totalOpenAttempts; } ///< 获取open()调用次数
    quint64 totalWrites() const { return m_totalWrites; } ///< 获取write()调用次数
    /** @brief 加入多播组 @param groupAddr 多播组地址(如"239.0.0.1") @return true=加入成功 */
    bool joinMulticastGroup(const QString& groupAddr);
    /** @brief 离开多播组 @param groupAddr 多播组地址 @return true=离开成功 */
    bool leaveMulticastGroup(const QString& groupAddr);
    void resetStats();                      ///< 重置所有统计数据

private slots:
    void onReadyRead();  ///< 数据到达回调，读取所有待处理的数据报
    void onError(QAbstractSocket::SocketError error); ///< 网络错误回调

private:
    /** @brief 将Qt网络错误码映射为中文描述 @param error Qt套接字错误码 @param systemError 系统错误描述 @return 中文错误描述 */
    static QString translateNetworkError(QAbstractSocket::SocketError error, const QString& systemError);
    void updateState(ConnectionState newState); ///< 更新连接状态并发射stateChanged信号
    // ---- 配置参数 ----
    quint16 m_localPort = 0;             ///< 本地绑定端口号
    QHostAddress m_remoteHost;           ///< 远程主机地址
    quint16 m_remotePort = 0;            ///< 远程端口号
    bool m_broadcast = false;            ///< 是否启用广播模式
    ConnectionState m_state = ConnectionState::Disconnected; ///< 当前连接状态
    // ---- 网络资源 ----
    QUdpSocket* m_socket = nullptr;      ///< UDP通信socket
    // ---- 统计计数器 ----
    quint64 m_totalDatagramsSent = 0;    ///< 已发送数据报总数
    quint64 m_totalDatagramsReceived = 0;///< 已接收数据报总数
    quint64 m_totalBytesSent = 0;        ///< 已发送字节总数
    quint64 m_totalBytesReceived = 0;    ///< 已接收字节总数
    quint64 m_errorCount = 0;            ///< 错误发生次数
    quint64 m_totalDatagramErrors = 0;   ///< 数据报发送/接收失败次数
    quint64 m_totalBroadcastsSent = 0;   ///< 广播数据报发送次数
    quint64 m_totalSocketErrors = 0;     ///< Socket级别错误次数
    quint64 m_totalMulticastJoins = 0;   ///< 累计加入多播组次数
    quint64 m_totalMulticastLeaves = 0;  ///< 累计离开多播组次数
    quint64 m_totalOpenAttempts = 0;     ///< 累计open()调用次数
    quint64 m_totalWrites = 0;           ///< 累计write()调用次数
};

#endif // UDPCONNECTION_H
