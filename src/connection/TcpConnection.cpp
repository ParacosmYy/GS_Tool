#include "connection/TcpConnection.h"
#include <QNetworkInterface>

TcpConnection::TcpConnection(QObject* parent)
    : IConnection(parent)
{
}

TcpConnection::~TcpConnection()
{
    close();
}

ConnectionType TcpConnection::type() const
{
    return ConnectionType::TcpClient;
}

QString TcpConnection::name() const
{
    if (m_mode == Client) {
        return QString("TCP:%1:%2").arg(m_host).arg(m_port);
    }
    return QString("TCP Server:%1").arg(m_port);
}

ConnectionState TcpConnection::state() const
{
    return m_state;
}

void TcpConnection::configure(const QVariantMap& params)
{
    m_host = params.value("host", "127.0.0.1").toString();
    m_port = static_cast<quint16>(params.value("port", 8080).toInt());
    m_mode = params.value("mode", "client").toString() == "server" ? Server : Client;
}

bool TcpConnection::open()
{
    if (m_mode == Client) {
        if (!m_socket) {
            m_socket = new QTcpSocket(this);
            connect(m_socket, &QTcpSocket::connected,
                    this, &TcpConnection::onSocketConnected);
            connect(m_socket, &QTcpSocket::disconnected,
                    this, &TcpConnection::onSocketDisconnected);
            connect(m_socket, &QTcpSocket::readyRead,
                    this, &TcpConnection::onSocketReadyRead);
            connect(m_socket, &QTcpSocket::errorOccurred,
                    this, &TcpConnection::onSocketError);
        }

        updateState(ConnectionState::Connecting);
        m_socket->connectToHost(m_host, m_port);
        // 异步连接，不等待结果
        return true;
    } else {
        // Server模式
        if (!m_server) {
            m_server = new QTcpServer(this);
            connect(m_server, &QTcpServer::newConnection,
                    this, &TcpConnection::onNewConnection);
        }

        if (!m_server->listen(QHostAddress::Any, m_port)) {
            emit errorOccurred(QString("TCP Server listen failed: %1").arg(m_server->errorString()));
            updateState(ConnectionState::Error);
            return false;
        }

        updateState(ConnectionState::Connected);
        return true;
    }
}

void TcpConnection::close()
{
    if (m_socket) {
        m_socket->disconnectFromHost();
        m_socket->deleteLater();
        m_socket = nullptr;
    }
    if (m_clientSocket) {
        m_clientSocket->disconnectFromHost();
        m_clientSocket->deleteLater();
        m_clientSocket = nullptr;
    }
    if (m_server) {
        m_server->close();
        m_server->deleteLater();
        m_server = nullptr;
    }
    updateState(ConnectionState::Disconnected);
}

qint64 TcpConnection::write(const QByteArray& data)
{
    QTcpSocket* target = nullptr;
    if (m_mode == Client) {
        target = m_socket;
    } else {
        target = m_clientSocket;
    }

    if (!target || target->state() != QAbstractSocket::ConnectedState) {
        return -1;
    }

    return target->write(data);
}

void TcpConnection::onSocketConnected()
{
    updateState(ConnectionState::Connected);
}

void TcpConnection::onSocketDisconnected()
{
    updateState(ConnectionState::Disconnected);
}

void TcpConnection::onSocketReadyRead()
{
    QTcpSocket* senderSock = qobject_cast<QTcpSocket*>(sender());
    if (!senderSock) return;

    QByteArray data = senderSock->readAll();
    if (!data.isEmpty()) {
        emit dataReceived(data);
    }
}

void TcpConnection::onSocketError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error);
    QTcpSocket* sock = qobject_cast<QTcpSocket*>(sender());
    QString msg = sock ? sock->errorString() : "Unknown TCP error";
    emit errorOccurred(msg);
    updateState(ConnectionState::Error);
}

void TcpConnection::onNewConnection()
{
    if (m_clientSocket) {
        m_clientSocket->disconnectFromHost();
        m_clientSocket->deleteLater();
    }

    m_clientSocket = m_server->nextPendingConnection();
    if (m_clientSocket) {
        connect(m_clientSocket, &QTcpSocket::readyRead,
                this, &TcpConnection::onSocketReadyRead);
        connect(m_clientSocket, &QTcpSocket::disconnected,
                this, [this]() {
                    m_clientSocket->deleteLater();
                    m_clientSocket = nullptr;
                });
        connect(m_clientSocket, &QTcpSocket::errorOccurred,
                this, &TcpConnection::onSocketError);
        updateState(ConnectionState::Connected);
    }
}

void TcpConnection::updateState(ConnectionState newState)
{
    if (m_state != newState) {
        m_state = newState;
        emit stateChanged(newState);
    }
}
