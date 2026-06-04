/**
 * @file UdpConnection.cpp
 * @brief UDP连接实现 - 封装 QUdpSocket 的无连接数据报通信
 *
 * 支持点对点UDP和广播模式:
 *   - 点对点: 绑定本地端口，向指定远程主机:端口发送数据报
 *   - 广播: 绑定本地端口，向广播地址发送数据报
 *   - 接收: 读取所有到达本地端口的数据报并转发
 */

#include "connection/network/UdpConnection.h"
#include "shared/ConnectionConstants.h"
#include <QVariant>

/** @brief 构造UDP连接，初始化内部socket为空 @param parent 父对象 */
UdpConnection::UdpConnection(QObject* parent)
    : IConnection(parent)
{
}

/** @brief 析构函数，静默关闭socket(不发射stateChanged信号，避免析构期间回调) */
UdpConnection::~UdpConnection()
{
    // 析构时仅释放资源，不发射信号(避免析构期间回调)
    if (m_socket) {
        m_socket->close();
        m_socket = nullptr;  // deleteLater在析构中无效，直接置空
    }
    m_state = ConnectionState::Disconnected;
}

/** @brief 返回连接类型(Udp) @return ConnectionType::Udp */
ConnectionType UdpConnection::type() const
{
    return ConnectionType::Udp;
}

/** @brief 返回连接名称(格式: UDP:localPort→remoteHost:remotePort 或 UDP:port:broadcast) @return 连接名称字符串 */
QString UdpConnection::name() const
{
    if (m_broadcast) {
        return QString("UDP:%1:broadcast").arg(m_localPort);
    }
    return QString("UDP:%1→%2:%3")
        .arg(m_localPort)
        .arg(m_remoteHost.toString())
        .arg(m_remotePort);
}

/** @brief 返回当前连接状态 @return ConnectionState枚举值 */
ConnectionState UdpConnection::state() const
{
    return m_state;
}

/** @brief 通过参数映射配置UDP连接参数 @param params 参数映射: localPort(本地端口)/remoteHost(远程主机)/remotePort(远程端口)/broadcast(广播模式) */
void UdpConnection::configure(const QVariantMap& params)
{
    m_localPort = static_cast<quint16>(params.value("localPort", QVariant(0)).toInt());
    m_remoteHost = QHostAddress(params.value("remoteHost", QVariant(ConnectionDefaults::kDefaultHost)).toString());
    m_remotePort = static_cast<quint16>(params.value("remotePort", QVariant(ConnectionDefaults::kDefaultPort)).toInt());
    m_broadcast = params.value("broadcast", QVariant(false)).toBool();
}

/** @brief 打开UDP连接，创建QUdpSocket并绑定到本地端口，广播模式绑定AnyIPv4，点对点绑定Any @return true=绑定成功, false=绑定失败 */
bool UdpConnection::open()
{
    // 已连接时先关闭旧socket，防止重复绑定导致 bind 失败
    if (m_socket) {
        m_socket->close();
        m_socket->deleteLater();
        m_socket = nullptr;
    }

    m_socket = new QUdpSocket(this);
    connect(m_socket, &QUdpSocket::readyRead,
            this, &UdpConnection::onReadyRead);
    connect(m_socket, &QUdpSocket::errorOccurred,
            this, &UdpConnection::onError);

    // 绑定本地端口（0=自动选择）
    QHostAddress bindAddr = m_broadcast ? QHostAddress::AnyIPv4 : QHostAddress::Any;
    if (!m_socket->bind(bindAddr, m_localPort)) {
        ++m_errorCount;  // 绑定失败计为错误
        emit errorOccurred(tr("UDP绑定端口失败: %1").arg(m_socket->errorString()));
        updateState(ConnectionState::Error);
        return false;
    }

    updateState(ConnectionState::Connected);
    return true;
}

/** @brief 关闭UDP连接，释放socket资源并重置状态为Disconnected */
void UdpConnection::close()
{
    if (m_socket) {
        m_socket->close();
        m_socket->deleteLater();
        m_socket = nullptr;
    }
    updateState(ConnectionState::Disconnected);
}

/** @brief 发送数据报到远程主机或广播地址 @param data 待发送的字节数据 @return 实际发送的字节数，-1表示未连接或发送失败 */
qint64 UdpConnection::write(const QByteArray& data)
{
    if (!m_socket || m_state != ConnectionState::Connected) {
        return -1;
    }

    qint64 written = 0;
    if (m_broadcast) {
        QHostAddress broadcastAddr = QHostAddress::Broadcast;
        written = m_socket->writeDatagram(data, broadcastAddr, m_remotePort);
    } else {
        written = m_socket->writeDatagram(data, m_remoteHost, m_remotePort);
    }

    if (written > 0) {
        ++m_totalDatagramsSent;
        m_totalBytesSent += static_cast<quint64>(written);
        if (m_broadcast) {
            ++m_totalBroadcastsSent;  // 广播数据报发送计数
        }
        emit bytesWritten(written);
    } else if (written < 0) {
        ++m_errorCount;
        ++m_totalDatagramErrors;  // 数据报发送失败计数
        emit errorOccurred(tr("UDP发送失败: %1").arg(m_socket->errorString()));
    }
    return written;
}

// 错误处理/数据接收/状态更新见 UdpConnectionHandlers.cpp
// 多播组管理/统计getter/resetStats见 UdpConnectionStats.cpp

