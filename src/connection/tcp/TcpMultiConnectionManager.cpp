/**
 * @file TcpMultiConnectionManager.cpp
 * @brief TCP多连接管理器实现
 */

#include "connection/tcp/TcpMultiConnectionManager.h"
#include <QHostAddress>

/**
 * @brief 构造函数
 * @param parent 父对象
 */
TcpMultiConnectionManager::TcpMultiConnectionManager(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 析构函数 - 关闭所有连接
 */
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

/**
 * @brief 添加新的TCP连接
 * @param host 目标主机地址
 * @param port 目标端口号
 * @return 新连接ID，-1表示失败
 */
int TcpMultiConnectionManager::addConnection(const QString& host, int port)
{
    auto* socket = new QTcpSocket(this);
    int id = m_nextId++;

    m_connections[id] = socket;
    m_hosts[id] = host;
    m_ports[id] = port;

    /// 绑定socket信号，使用lambda传递id
    connect(socket, &QTcpSocket::readyRead,
            this, &TcpMultiConnectionManager::onReadyRead);
    connect(socket, &QTcpSocket::disconnected,
            this, &TcpMultiConnectionManager::onDisconnected);
    connect(socket, &QAbstractSocket::errorOccurred,
            this, &TcpMultiConnectionManager::onError);

    /// 发起TCP连接
    socket->connectToHost(QHostAddress(host), static_cast<quint16>(port));
    emit connectionAdded(id, host, port);
    return id;
}

/**
 * @brief 移除指定连接
 * @param id 连接ID
 */
void TcpMultiConnectionManager::removeConnection(int id)
{
    if (!m_connections.contains(id)) return;

    QTcpSocket* socket = m_connections.take(id);
    m_hosts.remove(id);
    m_ports.remove(id);

    if (socket) {
        socket->disconnectFromHost();
        socket->deleteLater();
    }
    emit connectionRemoved(id);
}

/**
 * @brief 获取当前连接数量
 * @return 活跃连接数
 */
int TcpMultiConnectionManager::connectionCount() const
{
    return m_connections.size();
}

/**
 * @brief 向所有连接发送数据
 * @param data 待发送的字节数据
 * @return 成功发送的连接数
 */
int TcpMultiConnectionManager::sendToAll(const QByteArray& data)
{
    int count = 0;
    for (auto it = m_connections.begin(); it != m_connections.end(); ++it) {
        QTcpSocket* socket = it.value();
        if (socket && socket->state() == QAbstractSocket::ConnectedState) {
            qint64 written = socket->write(data);
            if (written > 0) {
                socket->flush();
                count++;
            }
        }
    }
    return count;
}

/**
 * @brief 获取连接的主机地址
 * @param id 连接ID
 * @return 主机地址字符串
 */
QString TcpMultiConnectionManager::connectionHost(int id) const
{
    return m_hosts.value(id, QString());
}

/**
 * @brief 获取连接的端口号
 * @param id 连接ID
 * @return 端口号，-1表示无效
 */
int TcpMultiConnectionManager::connectionPort(int id) const
{
    return m_ports.value(id, -1);
}

/**
 * @brief socket数据到达回调 - 根据sender识别连接ID
 */
void TcpMultiConnectionManager::onReadyRead()
{
    auto* socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;

    int id = idForSocket(socket);
    if (id < 0) return;

    QByteArray data = socket->readAll();
    if (!data.isEmpty()) {
        emit dataReceived(id, data);
    }
}

/**
 * @brief socket断开回调 - 自动移除连接
 */
void TcpMultiConnectionManager::onDisconnected()
{
    auto* socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;

    int id = idForSocket(socket);
    if (id >= 0) {
        removeConnection(id);
    }
}

/**
 * @brief socket错误回调
 * @param error socket错误码
 */
void TcpMultiConnectionManager::onError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error)
    auto* socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;

    int id = idForSocket(socket);
    if (id >= 0) {
        emit connectionError(id, socket->errorString());
    }
}

/**
 * @brief 查找socket对应的连接ID
 * @param socket 目标socket
 * @return 连接ID，-1表示未找到
 */
int TcpMultiConnectionManager::idForSocket(QTcpSocket* socket) const
{
    for (auto it = m_connections.constBegin(); it != m_connections.constEnd(); ++it) {
        if (it.value() == socket) {
            return it.key();
        }
    }
    return -1;
}
