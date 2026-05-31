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
        emit errorOccurred(QString("UDP bind failed: %1").arg(m_socket->errorString()));
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
    Q_UNUSED(error);
    QString msg = m_socket ? m_socket->errorString() : "Unknown UDP error";
    emit errorOccurred(msg);
    updateState(ConnectionState::Error);
}

void UdpConnection::updateState(ConnectionState newState)
{
    if (m_state != newState) {
        m_state = newState;
        emit stateChanged(newState);
    }
}
