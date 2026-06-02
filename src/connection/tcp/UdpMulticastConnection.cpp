/**
 * @file UdpMulticastConnection.cpp
 * @brief UDP组播连接实现
 */

#include "connection/tcp/UdpMulticastConnection.h"
#include <QNetworkInterface>

/**
 * @brief 构造函数
 * @param parent 父对象
 */
UdpMulticastConnection::UdpMulticastConnection(QObject* parent)
    : IConnection(parent)
{
}

/**
 * @brief 析构函数 - 关闭连接
 */
UdpMulticastConnection::~UdpMulticastConnection()
{
    close();
}

/**
 * @brief 获取连接类型
 * @return UDP类型
 */
ConnectionType UdpMulticastConnection::type() const
{
    return ConnectionType::Udp;
}

/**
 * @brief 获取连接显示名称
 * @return "Multicast:组地址:端口" 格式
 */
QString UdpMulticastConnection::name() const
{
    if (m_state == ConnectionState::Connected) {
        return QString("Multicast:%1:%2").arg(m_groupAddress.toString()).arg(m_localPort);
    }
    return tr("UDP Multicast (未连接)");
}

/**
 * @brief 获取当前状态
 */
ConnectionState UdpMulticastConnection::state() const
{
    return m_state;
}

/**
 * @brief 初始化socket(懒创建)
 */
void UdpMulticastConnection::ensureSocket()
{
    if (m_socket) return;

    m_socket = new QUdpSocket(this);
    connect(m_socket, &QUdpSocket::readyRead,
            this, &UdpMulticastConnection::onReadyRead);
    connect(m_socket, &QUdpSocket::errorOccurred,
            this, &UdpMulticastConnection::onError);
}

/**
 * @brief 打开连接 - 绑定本地端口并加入组播组
 * @return true=成功
 */
bool UdpMulticastConnection::open()
{
    ensureSocket();

    /// 绑定到本地端口(ShareAddress允许多个socket绑定同一端口)
    if (!m_socket->bind(QHostAddress::AnyIPv4, m_localPort,
                        QAbstractSocket::ShareAddress | QAbstractSocket::ReuseAddressHint)) {
        emit errorOccurred(tr("UDP绑定失败: %1").arg(m_socket->errorString()));
        updateState(ConnectionState::Error);
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

/**
 * @brief 关闭连接 - 离开组播组并释放socket
 */
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

/**
 * @brief 发送组播数据
 * @param data 待发送数据
 * @return 发送字节数
 */
qint64 UdpMulticastConnection::write(const QByteArray& data)
{
    if (!m_socket || m_state != ConnectionState::Connected) {
        return -1;
    }

    quint16 destPort = m_remotePort > 0 ? m_remotePort : m_localPort;
    qint64 written = m_socket->writeDatagram(data, m_groupAddress, destPort);
    if (written > 0) {
        emit bytesWritten(written);
    }
    return written;
}

/**
 * @brief 配置连接参数
 * @param params 参数映射:
 *   - "groupAddress": QString (组播地址)
 *   - "localPort": int
 *   - "remotePort": int
 */
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

/**
 * @brief 加入组播组
 * @param groupAddress 组播地址
 */
void UdpMulticastConnection::joinGroup(const QHostAddress& groupAddress)
{
    if (!m_socket) return;

    if (m_usingCustomInterface && m_multicastInterface.isValid()) {
        if (!m_socket->joinMulticastGroup(groupAddress, m_multicastInterface)) {
            emit errorOccurred(tr("加入组播组失败: %1").arg(m_socket->errorString()));
        }
    } else {
        if (!m_socket->joinMulticastGroup(groupAddress)) {
            emit errorOccurred(tr("加入组播组失败: %1").arg(m_socket->errorString()));
        }
    }
}

/**
 * @brief 离开组播组
 * @param groupAddress 组播地址
 */
void UdpMulticastConnection::leaveGroup(const QHostAddress& groupAddress)
{
    if (!m_socket) return;

    if (m_usingCustomInterface && m_multicastInterface.isValid()) {
        m_socket->leaveMulticastGroup(groupAddress, m_multicastInterface);
    } else {
        m_socket->leaveMulticastGroup(groupAddress);
    }
}

/**
 * @brief 设置组播网络接口
 * @param interfaceName 网络接口名称
 */
void UdpMulticastConnection::setMulticastInterface(const QString& interfaceName)
{
    for (const QNetworkInterface& iface : QNetworkInterface::allInterfaces()) {
        if (iface.humanReadableName() == interfaceName) {
            m_multicastInterface = iface;
            m_usingCustomInterface = true;
            return;
        }
    }
    m_usingCustomInterface = false;
}

/**
 * @brief 数据到达回调
 */
void UdpMulticastConnection::onReadyRead()
{
    if (!m_socket) return;

    while (m_socket->hasPendingDatagrams()) {
        QByteArray buffer;
        buffer.resize(static_cast<int>(m_socket->pendingDatagramSize()));
        QHostAddress sender;
        quint16 senderPort = 0;

        qint64 size = m_socket->readDatagram(buffer.data(), buffer.size(),
                                              &sender, &senderPort);
        if (size > 0) {
            buffer.resize(static_cast<int>(size));
            emit dataReceived(buffer);
        }
    }
}

/**
 * @brief 网络错误回调
 */
void UdpMulticastConnection::onError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error)
    if (m_socket) {
        emit errorOccurred(m_socket->errorString());
    }
}

/**
 * @brief 更新连接状态
 */
void UdpMulticastConnection::updateState(ConnectionState newState)
{
    if (m_state != newState) {
        m_state = newState;
        emit stateChanged(newState);
    }
}
