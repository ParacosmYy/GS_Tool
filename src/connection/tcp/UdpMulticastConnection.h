/**
 * @file UdpMulticastConnection.h
 * @brief UDP组播连接 - 支持UDP组播(多播)数据收发
 *
 * 职责:
 *   1. 加入/离开UDP组播组
 *   2. 收发组播数据报
 *   3. 复用IConnection抽象接口
 *
 * 协作关系:
 *   - ConnectionFactory: 通过工厂创建实例
 *   - ConnectionController: 管理连接生命周期
 */

#ifndef UDPMULTICASTCONNECTION_H
#define UDPMULTICASTCONNECTION_H

#include "connection/interface/IConnection.h"
#include <QUdpSocket>
#include <QHostAddress>
#include <QNetworkInterface>

/**
 * @brief UDP组播连接实现
 *
 * 支持加入指定组播组进行数据收发，适用于局域网设备发现、
 * 固件批量分发等一对多通信场景。
 */
class UdpMulticastConnection : public IConnection {
    Q_OBJECT

public:
    /** @brief 构造函数
     * @param parent 父对象
     */
    explicit UdpMulticastConnection(QObject* parent = nullptr);

    /** @brief 析构，关闭连接 */
    ~UdpMulticastConnection() override;

    // ---- IConnection接口实现 ----
    ConnectionType type() const override;              ///< 返回连接类型(UdpMulticast)
    QString name() const override;                     ///< 返回连接显示名称
    ConnectionState state() const override;            ///< 返回当前连接状态
    bool open() override;                              ///< 绑定本地端口并加入组播组
    void close() override;                             ///< 离开组播组并关闭socket
    qint64 write(const QByteArray& data) override;     ///< 发送组播数据报
    void configure(const QVariantMap& params) override; ///< 配置组播参数(groupAddress/port等)

    // ---- UDP组播特有接口 ----

    /**
     * @brief 加入组播组
     * @param groupAddress 组播地址(如224.0.0.1)
     */
    void joinGroup(const QHostAddress& groupAddress);

    /**
     * @brief 离开组播组
     * @param groupAddress 组播地址
     */
    void leaveGroup(const QHostAddress& groupAddress);

    /**
     * @brief 设置组播数据发送使用的网络接口
     * @param interfaceName 网络接口名称
     */
    void setMulticastInterface(const QString& interfaceName);

    /** @brief 获取已发送数据报计数 */
    quint64 datagramsSent() const;

    /** @brief 获取已接收数据报计数 */
    quint64 datagramsReceived() const;

    /** @brief 获取累计发送字节数 */
    qint64 totalBytesSent() const;

    /** @brief 获取累计接收字节数 */
    qint64 totalBytesReceived() const;

    /** @brief 获取组播组加入总次数 */
    quint64 totalJoins() const;

    /** @brief 获取组播组离开总次数 */
    quint64 totalLeaves() const;

    /** @brief 重置统计数据 */
    void resetStatistics();

private slots:
    /** @brief 数据到达回调 */
    void onReadyRead();

    /** @brief 网络错误回调 */
    void onError(QAbstractSocket::SocketError error);

private:
    /** @brief 更新连接状态 */
    void updateState(ConnectionState newState);

    /** @brief 初始化socket(懒创建) */
    void ensureSocket();

    // ---- 配置参数 ----
    QHostAddress m_groupAddress;                     ///< 组播组地址
    quint16 m_localPort = 0;                         ///< 本地绑定端口
    quint16 m_remotePort = 0;                        ///< 远程端口
    QNetworkInterface m_multicastInterface;           ///< 组播网络接口
    bool m_usingCustomInterface = false;              ///< 是否使用自定义接口
    ConnectionState m_state = ConnectionState::Disconnected; ///< 当前状态

    // ---- 网络资源 ----
    QUdpSocket* m_socket = nullptr;                  ///< UDP socket

    /** @brief 已发送数据报计数 */
    quint64 m_dgramsSent = 0;
    /** @brief 已接收数据报计数 */
    quint64 m_dgramsRecv = 0;
    /** @brief 累计发送字节数 */
    qint64 m_txBytes = 0;
    /** @brief 累计接收字节数 */
    qint64 m_rxBytes = 0;
    /** @brief 组播组加入总次数 */
    quint64 m_totalJoins = 0;
    /** @brief 组播组离开总次数 */
    quint64 m_totalLeaves = 0;
};

#endif // UDPMULTICASTCONNECTION_H
