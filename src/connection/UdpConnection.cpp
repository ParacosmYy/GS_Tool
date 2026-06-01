/**
 * @file UdpConnection.cpp
 * @brief UDP连接实现 - 封装 QUdpSocket 的无连接数据报通信
 *
 * 支持点对点UDP和广播模式:
 *   - 点对点: 绑定本地端口，向指定远程主机:端口发送数据报
 *   - 广播: 绑定本地端口，向广播地址发送数据报
 *   - 接收: 读取所有到达本地端口的数据报并转发
 */

#include "connection/UdpConnection.h"
#include "core/Constants.h"
#include <QVariant>

/** @brief 构造UDP连接(初始化QUdpSocket) @param parent 父对象 */
UdpConnection::UdpConnection(QObject* parent)
    : IConnection(parent)
{
}

/** @brief 析构函数，静默关闭(不发射stateChanged信号) */
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

/** @brief 返回连接名称(格式: UDP:localPort→remoteHost:remotePort 或 UDP:port:broadcast) @return 连接名称 */
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

/** @brief 返回当前连接状态 */
ConnectionState UdpConnection::state() const
{
    return m_state;
}

/**
 * @brief 通过参数映射配置UDP连接参数
 * @param params 参数映射: localPort(本地端口)/remoteHost(远程主机)/remotePort(远程端口)/broadcast(广播模式)
 */
void UdpConnection::configure(const QVariantMap& params)
{
    m_localPort = static_cast<quint16>(params.value("localPort", QVariant(0)).toInt());
    m_remoteHost = QHostAddress(params.value("remoteHost", QVariant(ConnectionDefaults::kDefaultHost)).toString());
    m_remotePort = static_cast<quint16>(params.value("remotePort", QVariant(ConnectionDefaults::kDefaultPort)).toInt());
    m_broadcast = params.value("broadcast", QVariant(false)).toBool();
}

/**
 * @brief 打开UDP连接
 *
 * 创建QUdpSocket并绑定到本地端口。已连接时先关闭旧socket防止重复绑定。
 * 广播模式绑定到AnyIPv4，点对点模式绑定到Any。
 * @return true=绑定成功, false=绑定失败
 */
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
        emit errorOccurred(tr("UDP绑定端口失败: %1").arg(m_socket->errorString()));
        updateState(ConnectionState::Error);
        return false;
    }

    updateState(ConnectionState::Connected);
    return true;
}

/** @brief 关闭UDP连接，释放socket资源并重置状态 */
void UdpConnection::close()
{
    if (m_socket) {
        m_socket->close();
        m_socket->deleteLater();
        m_socket = nullptr;
    }
    updateState(ConnectionState::Disconnected);
}

/**
 * @brief 发送数据报到远程主机或广播地址
 * @param data 待发送的字节数据
 * @return 实际发送的字节数，-1表示未连接或发送失败
 */
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
        emit bytesWritten(written);
    } else if (written < 0) {
        emit errorOccurred(tr("UDP发送失败: %1").arg(m_socket->errorString()));
    }
    return written;
}

/** @brief readyRead信号处理: 读取所有到达的数据报并转发给上层 */
void UdpConnection::onReadyRead()
{
    if (!m_socket) return;

    while (m_socket->hasPendingDatagrams()) {
        QByteArray buffer;
        buffer.resize(static_cast<int>(m_socket->pendingDatagramSize()));
        QHostAddress senderAddr;
        quint16 senderPort;
        m_socket->readDatagram(buffer.data(), buffer.size(), &senderAddr, &senderPort);
        if (!buffer.isEmpty()) {
            emit dataReceived(buffer);
        }
    }
}

/**
 * @brief socket错误处理
 * @param error Qt网络错误码，翻译为中文描述后通知上层
 */
void UdpConnection::onError(QAbstractSocket::SocketError error)
{
    QString systemError = m_socket ? m_socket->errorString() : QString();
    emit errorOccurred(translateNetworkError(error, systemError));
    updateState(ConnectionState::Error);
}

/**
 * @brief 将网络错误码翻译为中文描述
 * @param error Qt网络错误枚举
 * @param systemError 系统级错误描述字符串
 * @return 人类可读的中文错误描述
 */
QString UdpConnection::translateNetworkError(QAbstractSocket::SocketError error,
                                              const QString& systemError)
{
    switch (error) {
    case QAbstractSocket::ConnectionRefusedError:
        return UdpConnection::tr("连接被拒绝，请检查目标地址和端口是否正确");
    case QAbstractSocket::RemoteHostClosedError:
        return UdpConnection::tr("远程主机已关闭连接");
    case QAbstractSocket::HostNotFoundError:
        return UdpConnection::tr("无法解析主机名，请检查地址是否正确");
    case QAbstractSocket::NetworkError:
        return UdpConnection::tr("网络异常，请检查网络连接");
    case QAbstractSocket::SocketAccessError:
        return UdpConnection::tr("套接字访问被拒绝，权限不足");
    case QAbstractSocket::SocketResourceError:
        return UdpConnection::tr("系统资源不足，无法创建套接字");
    case QAbstractSocket::SocketTimeoutError:
        return UdpConnection::tr("通信超时，请检查目标主机是否可达");
    case QAbstractSocket::DatagramTooLargeError:
        return UdpConnection::tr("数据报过大，超出系统限制");
    case QAbstractSocket::AddressInUseError:
        return UdpConnection::tr("地址/端口已被占用，请更换端口");
    case QAbstractSocket::SocketAddressNotAvailableError:
        return UdpConnection::tr("请求的地址不可用");
    case QAbstractSocket::UnsupportedSocketOperationError:
        return UdpConnection::tr("不支持的操作");
    case QAbstractSocket::ProxyAuthenticationRequiredError:
        return UdpConnection::tr("代理服务器需要认证");
    case QAbstractSocket::TemporaryError:
        return UdpConnection::tr("临时错误，请稍后重试");
    default:
        break;
    }
    if (systemError.isEmpty())
        return UdpConnection::tr("未知UDP错误");
    return UdpConnection::tr("UDP错误: %1").arg(systemError);
}

/** @brief 更新连接状态（仅当状态变化时发射信号） */
void UdpConnection::updateState(ConnectionState newState)
{
    if (m_state != newState) {
        m_state = newState;
        emit stateChanged(newState);
    }
}
