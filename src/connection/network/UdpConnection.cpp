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

/** @brief readyRead信号处理，读取所有到达的数据报并转发给上层，同时累计接收字节和数据报统计 */
void UdpConnection::onReadyRead()
{
    if (!m_socket) return;

    while (m_socket->hasPendingDatagrams()) {
        QByteArray buffer;
        buffer.resize(static_cast<int>(m_socket->pendingDatagramSize()));
        QHostAddress senderAddr;
        quint16 senderPort;
        qint64 bytesRead = m_socket->readDatagram(buffer.data(), buffer.size(),
                                                   &senderAddr, &senderPort);
        if (bytesRead < 0) {
            ++m_totalDatagramErrors;  // 数据报接收失败计数
            continue;
        }
        buffer.resize(static_cast<int>(bytesRead));
        ++m_totalDatagramsReceived;
        m_totalBytesReceived += static_cast<quint64>(bytesRead);
        if (!buffer.isEmpty()) {
            emit dataReceived(buffer);
        }
    }
}

/** @brief socket错误处理，递增错误计数并翻译错误码为中文描述后通知上层 @param error Qt网络错误码 */
void UdpConnection::onError(QAbstractSocket::SocketError error)
{
    ++m_errorCount;
    ++m_totalSocketErrors;  ///< 累计Socket错误次数
    QString systemError = m_socket ? m_socket->errorString() : QString();
    emit errorOccurred(translateNetworkError(error, systemError));
    updateState(ConnectionState::Error);
}

/** @brief 将网络错误码翻译为中文描述 @param error Qt网络错误枚举 @param systemError 系统级错误描述字符串 @return 人类可读的中文错误描述 */
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

/** @brief 更新连接状态(仅当状态变化时发射stateChanged信号) @param newState 新的连接状态 */
void UdpConnection::updateState(ConnectionState newState)
{
    if (m_state != newState) {
        m_state = newState;
        emit stateChanged(newState);
    }
}

// ---- 统计接口实现 ----

/** @brief 获取已发送数据报总数 @return 数据报发送总数 */
quint64 UdpConnection::totalDatagramsSent() const { return m_totalDatagramsSent; }

/** @brief 获取已接收数据报总数 @return 数据报接收总数 */
quint64 UdpConnection::totalDatagramsReceived() const { return m_totalDatagramsReceived; }

/** @brief 获取已发送字节总数 @return 发送字节总数 */
quint64 UdpConnection::totalBytesSent() const { return m_totalBytesSent; }

/** @brief 获取已接收字节总数 @return 接收字节总数 */
quint64 UdpConnection::totalBytesReceived() const { return m_totalBytesReceived; }

/** @brief 获取错误计数 @return 累计错误次数 */
quint64 UdpConnection::errorCount() const { return m_errorCount; }

/** @brief 重置所有统计数据(数据报/字节/错误/广播/Socket错误计数)为零 */
void UdpConnection::resetStats()
{
    m_totalDatagramsSent = 0;
    m_totalDatagramsReceived = 0;
    m_totalBytesSent = 0;
    m_totalBytesReceived = 0;
    m_errorCount = 0;
    m_totalDatagramErrors = 0;
    m_totalBroadcastsSent = 0;
    m_totalSocketErrors = 0;
}

