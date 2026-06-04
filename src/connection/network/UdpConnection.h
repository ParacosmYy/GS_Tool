/**
 * @file UdpConnection.h
 * @brief UDP连接实现 - 支持单播/广播的UDP数据报收发
 *
 * 职责:
 *   1. 单播模式: 向指定远程主机和端口发送UDP数据报
 *   2. 广播模式: 向子网广播地址发送UDP数据报
 *   3. 复用IConnection抽象接口，上层无需关心连接类型
 *
 * 设计模式: 策略模式 — UdpConnection是IConnection的一个具体策略实现
 *
 * 协作关系:
 *   - ConnectionFactory: 通过工厂创建UdpConnection实例
 *   - ConnectionController: 管理连接生命周期、状态分发
 *
 * 注意: UDP是无连接协议，此处的"Connected"状态表示socket已绑定本地端口
 */

#ifndef UDPCONNECTION_H
#define UDPCONNECTION_H

#include "connection/interface/IConnection.h"
#include <QUdpSocket>
#include <QHostAddress>

/**
 * @brief UDP连接实现 - 支持单播/广播数据报收发
 *
 * 单播: 向指定远程主机:端口发送数据报
 * 广播: 向255.255.255.255发送数据报，子网内所有设备可接收
 *
 * 网络特性:
 *   - 每次write()成功后发射bytesWritten信号，供发送统计
 *   - 写入失败时通过errorOccurred信号报告错误
 *   - readyRead时读取所有到达的数据报并转发为dataReceived信号
 */
class UdpConnection : public IConnection {
    Q_OBJECT

public:
    /** @brief 构造UDP连接
     * @param parent 父对象，用于QObject生命周期管理
     */
    explicit UdpConnection(QObject* parent = nullptr);

    /** @brief 析构，关闭socket并释放资源 */
    ~UdpConnection() override;

    /** @brief 返回连接类型(UDP) */
    ConnectionType type() const override;

    /** @brief 返回连接显示名称 */
    QString name() const override;

    /** @brief 返回当前连接状态 */
    ConnectionState state() const override;

    /** @brief 打开UDP连接(绑定本地端口)
     * @return true=绑定成功，false=绑定失败
     */
    bool open() override;

    /** @brief 关闭UDP连接，释放socket资源 */
    void close() override;

    /** @brief 发送UDP数据报
     * @param data 待发送的字节数据
     * @return 实际发送字节数，-1表示连接未就绪或发送失败
     *
     * 发送成功后发射bytesWritten信号，失败时发射errorOccurred信号
     */
    qint64 write(const QByteArray& data) override;

    /** @brief 配置UDP连接参数
     * @param params 参数映射:
     *   - "localPort": int (本地绑定端口号)
     *   - "remoteHost": QString (远程主机地址)
     *   - "remotePort": int (远程端口号)
     *   - "broadcast": bool (是否启用广播模式)
     */
    void configure(const QVariantMap& params) override;

    // ---- 统计接口 ----

    /** @brief 获取已发送数据报总数 */
    quint64 totalDatagramsSent() const;

    /** @brief 获取已接收数据报总数 */
    quint64 totalDatagramsReceived() const;

    /** @brief 获取已发送字节总数 */
    quint64 totalBytesSent() const;

    /** @brief 获取已接收字节总数 */
    quint64 totalBytesReceived() const;

    /** @brief 获取错误计数 */
    quint64 errorCount() const;

    /** @brief 获取累计错误次数(errorCount的别名) @return 累计错误次数 */
    quint64 totalErrors() const { return m_errorCount; }

    /** @brief 重置所有统计数据为零 */
    void resetStats();

private slots:
    /** @brief 数据到达回调，读取所有待处理的数据报 */
    void onReadyRead();

    /** @brief 网络错误回调 */
    void onError(QAbstractSocket::SocketError error);

private:
    /** @brief 将Qt网络错误码映射为中文描述
     * @param error Qt套接字错误码
     * @param systemError 系统错误描述字符串
     * @return 用户可读的中文错误描述
     */
    static QString translateNetworkError(QAbstractSocket::SocketError error,
                                         const QString& systemError);

    /** @brief 更新连接状态并发射stateChanged信号 */
    void updateState(ConnectionState newState);

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
};

#endif // UDPCONNECTION_H
