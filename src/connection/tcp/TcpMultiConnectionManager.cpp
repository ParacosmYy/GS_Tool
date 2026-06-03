/**
 * @file TcpMultiConnectionManager.cpp
 * @brief TCP多连接管理器实现
 */

#include "connection/tcp/TcpMultiConnectionManager.h"
#include <QHostAddress>

TcpMultiConnectionManager::TcpMultiConnectionManager(QObject* parent)
    : QObject(parent)
{
}

TcpMultiConnectionManager::~TcpMultiConnectionManager()
{
    for (auto it = m_connections.begin(); it != m_connections.end(); ++it) {
        QTcpSocket* socket = it.value();
        if (socket) {
            socket->disconnectFromHost();
            socket->deleteLater();
        }
    }
    m_connections.clear();
    m_hosts.clear();
    m_ports.clear();
}

int TcpMultiConnectionManager::addConnection(const QString& host, int port)
{
    auto* socket = new QTcpSocket(this);
    int id = m_nextId++;

    m_connections[id] = socket;
    m_hosts[id] = host;
    m_ports[id] = port;
    ++m_totalConnections;

    connect(socket, &QTcpSocket::readyRead,
            this, &TcpMultiConnectionManager::onReadyRead);
    connect(socket, &QTcpSocket::disconnected,
            this, &TcpMultiConnectionManager::onDisconnected);
    connect(socket, &QAbstractSocket::errorOccurred,
            this, &TcpMultiConnectionManager::onError);

    socket->connectToHost(QHostAddress(host), static_cast<quint16>(port));
    emit connectionAdded(id, host, port);
    return id;
}

void TcpMultiConnectionManager::removeConnection(int id)
{
    if (!m_connections.contains(id)) return;
    ++m_totalDisconnections;

    QTcpSocket* socket = m_connections.take(id);
    m_hosts.remove(id);
    m_ports.remove(id);

    if (socket) {
        socket->disconnectFromHost();
        socket->deleteLater();
    }
    emit connectionRemoved(id);
}

int TcpMultiConnectionManager::connectionCount() const
{
    return m_connections.size();
}

int TcpMultiConnectionManager::sendToAll(const QByteArray& data)
{
    int count = 0;
    for (auto it = m_connections.begin(); it != m_connections.end(); ++it) {
        QTcpSocket* socket = it.value();
        if (socket && socket->state() == QAbstractSocket::ConnectedState) {
            qint64 written = socket->write(data);
            if (written > 0) {
                m_totalBytesSent += static_cast<quint64>(written);
                socket->flush();
                count++;
            }
        }
    }
    return count;
}

QString TcpMultiConnectionManager::connectionHost(int id) const
{
    return m_hosts.value(id, QString());
}

int TcpMultiConnectionManager::connectionPort(int id) const
{
    return m_ports.value(id, -1);
}

quint64 TcpMultiConnectionManager::totalConnections() const { return m_totalConnections; }
quint64 TcpMultiConnectionManager::totalDisconnections() const { return m_totalDisconnections; }
quint64 TcpMultiConnectionManager::totalBytesSent() const { return m_totalBytesSent; }
quint64 TcpMultiConnectionManager::totalBytesReceived() const { return m_totalBytesReceived; }
quint64 TcpMultiConnectionManager::errorCount() const { return m_errorCount; }

void TcpMultiConnectionManager::resetConnectionStatistics()
{
    m_totalConnections = 0;
    m_totalDisconnections = 0;
    m_totalBytesSent = 0;
    m_totalBytesReceived = 0;
    m_errorCount = 0;
}

/** @brief socket数据到达回调 — 累计接收字节数 */
void TcpMultiConnectionManager::onReadyRead()
{
    auto* socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;

    int id = idForSocket(socket);
    if (id < 0) return;

    QByteArray data = socket->readAll();
    if (!data.isEmpty()) {
        m_totalBytesReceived += static_cast<quint64>(data.size());
        emit dataReceived(id, data);
    }
}

/** @brief socket断开回调 — 自动移除连接 */
void TcpMultiConnectionManager::onDisconnected()
{
    auto* socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;

    int id = idForSocket(socket);
    if (id >= 0) {
        removeConnection(id);
    }
}

/** @brief socket错误回调 — 累计错误计数 */
void TcpMultiConnectionManager::onError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error)
    auto* socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;

    int id = idForSocket(socket);
    if (id >= 0) {
        ++m_errorCount;
        emit connectionError(id, socket->errorString());
    }
}

int TcpMultiConnectionManager::idForSocket(QTcpSocket* socket) const
{
    for (auto it = m_connections.constBegin(); it != m_connections.constEnd(); ++it) {
        if (it.value() == socket) {
            return it.key();
        }
    }
    return -1;
}
