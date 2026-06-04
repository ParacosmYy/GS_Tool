/**
 * @file UdpMulticastConnection.cpp
 * @brief UDP组播连接实现
 */

#include "connection/tcp/UdpMulticastConnection.h"
#include <QNetworkInterface>

/** @brief 构造UDP组播连接对象 @param parent 父QObject指针 */
UdpMulticastConnection::UdpMulticastConnection(QObject* parent)
    : IConnection(parent)
{
}

/** @brief 析构UDP组播连接，离开组播组并关闭socket */
UdpMulticastConnection::~UdpMulticastConnection()
{
    close();
}

/** @brief 获取连接类型 @return ConnectionType::Udp */
ConnectionType UdpMulticastConnection::type() const
{
    return ConnectionType::Udp;
}

/** @brief 获取连接显示名称 @return 已连接时返回"组播:地址:端口"格式，否则返回"未连接" */
QString UdpMulticastConnection::name() const
{
    if (m_state == ConnectionState::Connected) {
        return tr("组播:%1:%2").arg(m_groupAddress.toString()).arg(m_localPort);
    }
    return tr("UDP Multicast (未连接)");
}

/** @brief 获取当前连接状态 @return 连接状态枚举值 */
ConnectionState UdpMulticastConnection::state() const
{
    return m_state;
}

/** @brief 懒创建QUdpSocket并连接readyRead/errorOccurred信号 */
void UdpMulticastConnection::ensureSocket()
{
    if (m_socket) return;

    m_socket = new QUdpSocket(this);
    connect(m_socket, &QUdpSocket::readyRead,
            this, &UdpMulticastConnection::onReadyRead);
    connect(m_socket, &QUdpSocket::errorOccurred,
            this, &UdpMulticastConnection::onError);
}

/** @brief 打开连接，绑定本地端口并加入组播组 @return true=绑定成功 */
bool UdpMulticastConnection::open()
{
    ++m_totalOpenAttempts;
    ensureSocket();

    /// 绑定到本地端口(ShareAddress允许多个socket绑定同一端口)
    if (!m_socket->bind(QHostAddress::AnyIPv4, m_localPort,
                        QAbstractSocket::ShareAddress | QAbstractSocket::ReuseAddressHint)) {
        emit errorOccurred(tr("UDP绑定失败: %1").arg(m_socket->errorString()));
        updateState(ConnectionState::Error);
        ++m_totalNetworkErrors;
        return false;
    }

    /// 设置组播网络接口(如已配置)
    if (m_usingCustomInterface && m_multicastInterface.isValid()) {
        m_socket->setMulticastInterface(m_multicastInterface);
    }

    /// 加入组播组
    if (!m_groupAddress.isNull()) {
        joinGroup(m_groupAddress);
    }

    updateState(ConnectionState::Connected);
    return true;
}

/** @brief 关闭连接，离开组播组并释放socket */
void UdpMulticastConnection::close()
{
    if (m_socket) {
        if (!m_groupAddress.isNull() &&
            m_socket->state() != QAbstractSocket::UnconnectedState) {
            leaveGroup(m_groupAddress);
        }
        m_socket->close();
        m_socket->deleteLater();
        m_socket = nullptr;
    }
    updateState(ConnectionState::Disconnected);
}

/** @brief 发送组播数据到组播组 @param data 待发送数据 @return 发送字节数，未连接返回-1 */
qint64 UdpMulticastConnection::write(const QByteArray& data)
{
    if (!m_socket || m_state != ConnectionState::Connected) {
        return -1;
    }

    quint16 destPort = m_remotePort > 0 ? m_remotePort : m_localPort;
    qint64 written = m_socket->writeDatagram(data, m_groupAddress, destPort);
    if (written > 0) {
        ++m_dgramsSent;
        m_txBytes += written;
        emit bytesWritten(written);
    }
    return written;
}

/** @brief 配置UDP组播参数(groupAddress/localPort/remotePort) @param params 参数映射 */
void UdpMulticastConnection::configure(const QVariantMap& params)
{
    if (params.contains("groupAddress")) {
        m_groupAddress = QHostAddress(params["groupAddress"].toString());
    }
    if (params.contains("localPort")) {
        m_localPort = static_cast<quint16>(params["localPort"].toInt());
    }
    if (params.contains("remotePort")) {
        m_remotePort = static_cast<quint16>(params["remotePort"].toInt());
    }
}

// 组播组管理/统计接口见 UdpMulticastGroup.cpp

// onReadyRead/onError/updateState已移至 UdpMulticastConnectionHandlers.cpp
// 组播组管理/统计接口见 UdpMulticastGroup.cpp
