/**
 * @file TcpServerConnection.cpp
 * @brief TCP服务器模式连接实现 - 多客户端TCP服务端
 */

#include "connection/tcp/TcpServerConnection.h"
#include <QMap>

/** @brief 构造TCP服务器连接 @param parent 父QObject指针 */
TcpServerConnection::TcpServerConnection(QObject* parent)
    : IConnection(parent)
{
}

/** @brief 析构TCP服务器连接，停止监听并释放所有客户端资源 */
TcpServerConnection::~TcpServerConnection()
{
    close();
}

/** @brief 获取连接类型 @return ConnectionType::TcpServer */
ConnectionType TcpServerConnection::type() const
{
    return ConnectionType::TcpServer;
}

/** @brief 获取连接显示名称 @return 已监听时返回"TCP服务器:端口"格式，否则返回"未监听" */
QString TcpServerConnection::name() const
{
    if (m_listening) {
        return tr("TCP服务器:%1").arg(m_listenPort);
    }
    return tr("TCP Server (未监听)");
}

/** @brief 获取当前连接状态 @return 连接状态枚举值 */
ConnectionState TcpServerConnection::state() const
{
    return m_state;
}

/** @brief 打开连接即开始监听 @return true=监听成功 */
bool TcpServerConnection::open()
{
    return listen(m_listenAddress, m_listenPort);
}

/** @brief 关闭连接即停止监听并断开所有客户端 */
void TcpServerConnection::close()
{
    stopListening();
}

/** @brief 广播发送数据到所有已连接客户端 @param data 待发送的字节数据 @return 成功发送的客户端数量 */
qint64 TcpServerConnection::write(const QByteArray& data)
{
    ++m_totalWrites;
    return broadcastToClients(data);
}

/** @brief 配置TCP服务器参数(listenAddress/port) @param params 参数映射 */
void TcpServerConnection::configure(const QVariantMap& params)
{
    if (params.contains("listenAddress")) {
        m_listenAddress = QHostAddress(params["listenAddress"].toString());
    }
    if (params.contains("port")) {
        m_listenPort = static_cast<quint16>(params["port"].toInt());
    }
}

/** @brief 开始监听指定地址和端口 @param address 监听地址 @param port 监听端口号 @return true=监听成功 */
bool TcpServerConnection::listen(const QHostAddress& address, int port)
{
    ++m_totalListenAttempts;
    if (m_listening) {
        return true;  ///< 已在监听，直接返回成功
    }

    m_server = new QTcpServer(this);
    connect(m_server, &QTcpServer::newConnection,
            this, &TcpServerConnection::onNewConnection);

    if (!m_server->listen(address, static_cast<quint16>(port))) {
        ++m_totalAcceptErrors;  // 监听失败计为accept错误
        emit errorOccurred(tr("监听失败: %1").arg(m_server->errorString()));
        m_server->deleteLater();
        m_server = nullptr;
        updateState(ConnectionState::Error);
        return false;
    }

    m_listenAddress = address;
    m_listenPort = static_cast<quint16>(port);
    m_listening = true;
    updateState(ConnectionState::Connected);
    return true;
}

/** @brief 停止监听，断开所有客户端并关闭服务器 */
void TcpServerConnection::stopListening()
{
    /// 断开所有客户端
    for (auto it = m_clients.begin(); it != m_clients.end(); ++it) {
        QTcpSocket* socket = it.value();
        if (socket) {
            socket->disconnectFromHost();
            if (socket->state() != QAbstractSocket::UnconnectedState) {
                socket->waitForDisconnected(1000);
            }
            socket->deleteLater();
        }
    }
    m_clients.clear();

    /// 关闭服务器
    if (m_server) {
        m_server->close();
        m_server->deleteLater();
        m_server = nullptr;
    }

    m_listening = false;
    updateState(ConnectionState::Disconnected);
}

/** @brief 获取已连接的客户端列表 @return 客户端"地址:端口"字符串列表 */
QStringList TcpServerConnection::connectedClients() const
{
    QStringList result;
    for (auto it = m_clients.constBegin(); it != m_clients.constEnd(); ++it) {
        QTcpSocket* socket = it.value();
        if (socket && socket->state() == QAbstractSocket::ConnectedState) {
            result.append(clientInfo(socket));
        }
    }
    return result;
}

/** @brief 向所有已连接客户端广播数据 @param data 待广播的字节数据 @return 成功发送的客户端数量 */
int TcpServerConnection::broadcastToClients(const QByteArray& data)
{
    int count = 0;
    for (auto it = m_clients.begin(); it != m_clients.end(); ++it) {
        QTcpSocket* socket = it.value();
        if (socket && socket->state() == QAbstractSocket::ConnectedState) {
            qint64 written = socket->write(data);
            if (written > 0) {
                m_totalTxBytes += written;
                socket->flush();
                emit bytesWritten(written);
                count++;
            }
        }
    }
    if (count > 0) {
        ++m_broadcastCount;
    }
    return count;
}

/** @brief 新客户端连接回调，注册socket并连接断开/数据到达/错误信号 */
void TcpServerConnection::onNewConnection()
{
    if (!m_server) return;

    while (m_server->hasPendingConnections()) {
        QTcpSocket* client = m_server->nextPendingConnection();
        if (!client) continue;

        qintptr sd = client->socketDescriptor();
        m_clients[sd] = client;

        connect(client, &QTcpSocket::disconnected,
                this, &TcpServerConnection::onClientDisconnected);
        connect(client, &QTcpSocket::readyRead,
                this, &TcpServerConnection::onClientReadyRead);
        connect(client, &QAbstractSocket::errorOccurred,
                this, [this](QAbstractSocket::SocketError err) {
                    Q_UNUSED(err)
                    auto* socket = qobject_cast<QTcpSocket*>(sender());
                    if (socket) {
                        ++m_totalAcceptErrors;  // 客户端socket错误计数
                        emit errorOccurred(tr("客户端错误: %1")
                            .arg(socket->errorString()));
                    }
                });

        QString info = clientInfo(client);
        ++m_totalClientCount;
        emit clientConnected(info);
    }
}

/** @brief 客户端断开回调，从映射表中移除并释放socket */
void TcpServerConnection::onClientDisconnected()
{
    auto* socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;

    QString info = clientInfo(socket);

    /* socket已断开，descriptor可能无效，遍历查找 */
    qintptr sd = -1;
    for (auto it = m_clients.begin(); it != m_clients.end(); ++it) {
        if (it.value() == socket) {
            sd = it.key();
            break;
        }
    }

    if (sd >= 0) {
        m_clients.remove(sd);
    }
    ++m_totalClientDisconnections;  // 客户端断开计数
    emit clientDisconnected(info);
    socket->deleteLater();
}

/** @brief 客户端数据到达回调，发射clientData和dataReceived信号 */
void TcpServerConnection::onClientReadyRead()
{
    auto* socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;

    QByteArray data = socket->readAll();
    if (data.isEmpty()) return;

    m_totalRxBytes += data.size();

    QString info = clientInfo(socket);
    emit clientData(info, data);
    /// 同时发射IConnection标准信号，便于上层统一接收
    emit dataReceived(data);
}

/** @brief 更新连接状态，状态变化时发射stateChanged信号 @param newState 新状态 */
void TcpServerConnection::updateState(ConnectionState newState)
{
    if (m_state != newState) {
        m_state = newState;
        emit stateChanged(newState);
    }
}

/** @brief 获取socket对应的客户端"地址:端口"标识 @param socket 客户端socket @return "地址:端口"格式字符串 */
QString TcpServerConnection::clientInfo(QTcpSocket* socket)
{
    if (!socket) return QString();
    return QString("%1:%2").arg(socket->peerAddress().toString()).arg(socket->peerPort());
}

/** @brief 获取历史累计连接客户端总数 @return 客户端连接总数 */
quint64 TcpServerConnection::totalClientCount() const
{
    return m_totalClientCount;
}

/** @brief 获取历史累计断开客户端总数 @return 客户端断开总数 */
quint64 TcpServerConnection::totalClientDisconnections() const
{
    return m_totalClientDisconnections;
}

/** @brief 获取已广播数据包总数 @return 广播次数 */
quint64 TcpServerConnection::broadcastCount() const
{
    return m_broadcastCount;
}

/** @brief 获取累计接收字节数 @return 接收字节总量 */
quint64 TcpServerConnection::totalBytesReceived() const
{
    return m_totalRxBytes;
}

/** @brief 获取累计发送字节数 @return 发送字节总量 */
quint64 TcpServerConnection::totalBytesSent() const
{
    return m_totalTxBytes;
}

/** @brief 获取累计accept错误次数 @return accept错误总数 */
quint64 TcpServerConnection::totalAcceptErrors() const
{
    return m_totalAcceptErrors;
}

/** @brief 重置所有统计计数器为零 */
void TcpServerConnection::resetStatistics()
{
    m_totalClientCount = 0;
    m_totalClientDisconnections = 0;
    m_broadcastCount = 0;
    m_totalRxBytes = 0;
    m_totalTxBytes = 0;
    m_totalAcceptErrors = 0;
    m_totalListenAttempts = 0;
    m_totalWrites = 0;
}
