/**
 * @file UdpMulticastConnection.cpp
 * @brief UDP组播连接实现 - 骨架
 */

#include "connection/tcp/UdpMulticastConnection.h"

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
 * @brief 打开连接 - 绑定本地端口并加入组播组
 * @return true=成功
 */
bool UdpMulticastConnection::open()
{
    // TODO: 绑定本地端口，加入组播组
    updateState(ConnectionState::Connected);
    return true;
}

/**
 * @brief 关闭连接 - 离开组播组并释放socket
 */
void UdpMulticastConnection::close()
{
    if (m_socket) {
        m_socket->close();
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
    Q_UNUSED(data)
    // TODO: 向组播组地址发送数据报
    return -1;
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
    Q_UNUSED(groupAddress)
    // TODO: 调用m_socket->joinMulticastGroup()
}

/**
 * @brief 离开组播组
 * @param groupAddress 组播地址
 */
void UdpMulticastConnection::leaveGroup(const QHostAddress& groupAddress)
{
    Q_UNUSED(groupAddress)
    // TODO: 调用m_socket->leaveMulticastGroup()
}

/**
 * @brief 设置组播网络接口
 * @param interfaceName 网络接口名称
 */
void UdpMulticastConnection::setMulticastInterface(const QString& interfaceName)
{
    Q_UNUSED(interfaceName)
    // TODO: 查找对应网络接口并设置为组播接口
}

/**
 * @brief 数据到达回调
 */
void UdpMulticastConnection::onReadyRead()
{
    // TODO: 读取数据报并发射dataReceived信号
}

/**
 * @brief 网络错误回调
 */
void UdpMulticastConnection::onError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error)
    // TODO: 翻译错误并发射errorOccurred信号
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
