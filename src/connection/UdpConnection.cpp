#include "connection/UdpConnection.h"
#include <QVariant>

UdpConnection::UdpConnection(QObject* parent)
    : IConnection(parent)
{
}

UdpConnection::~UdpConnection()
{
    close();
}

ConnectionType UdpConnection::type() const
{
    return ConnectionType::Udp;
}

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

ConnectionState UdpConnection::state() const
{
    return m_state;
}

void UdpConnection::configure(const QVariantMap& params)
{
    m_localPort = static_cast<quint16>(params.value("localPort", QVariant(0)).toInt());
    m_remoteHost = QHostAddress(params.value("remoteHost", QVariant("127.0.0.1")).toString());
    m_remotePort = static_cast<quint16>(params.value("remotePort", QVariant(8080)).toInt());
    m_broadcast = params.value("broadcast", QVariant(false)).toBool();
}

bool UdpConnection::open()
{
    if (!m_socket) {
        m_socket = new QUdpSocket(this);
        connect(m_socket, &QUdpSocket::readyRead,
                this, &UdpConnection::onReadyRead);
        connect(m_socket, &QUdpSocket::errorOccurred,
                this, &UdpConnection::onError);
    }

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

void UdpConnection::close()
{
    if (m_socket) {
        m_socket->close();
        m_socket->deleteLater();
        m_socket = nullptr;
    }
    updateState(ConnectionState::Disconnected);
}

qint64 UdpConnection::write(const QByteArray& data)
{
    if (!m_socket || m_state != ConnectionState::Connected) {
        return -1;
    }

    if (m_broadcast) {
        QHostAddress broadcastAddr = QHostAddress::Broadcast;
        return m_socket->writeDatagram(data, broadcastAddr, m_remotePort);
    }

    return m_socket->writeDatagram(data, m_remoteHost, m_remotePort);
}

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

void UdpConnection::onError(QAbstractSocket::SocketError error)
{
    QString systemError = m_socket ? m_socket->errorString() : QString();
    emit errorOccurred(translateNetworkError(error, systemError));
    updateState(ConnectionState::Error);
}

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

void UdpConnection::updateState(ConnectionState newState)
{
    if (m_state != newState) {
        m_state = newState;
        emit stateChanged(newState);
    }
}
