/**
 * @file TcpServerConnection.cpp
 * @brief TCP服务器模式连接实现 - 多客户端TCP服务端骨架
 */

#include "connection/tcp/TcpServerConnection.h"

/**
 * @brief 构造函数 - 初始化TCP服务器
 * @param parent 父对象
 */
TcpServerConnection::TcpServerConnection(QObject* parent)
    : IConnection(parent)
{
}

/**
 * @brief 析构函数 - 关闭连接释放资源
 */
TcpServerConnection::~TcpServerConnection()
{
    close();
}

/**
 * @brief 获取连接类型
 * @return TcpServer类型
 */
ConnectionType TcpServerConnection::type() const
{
    return ConnectionType::TcpServer;
}

/**
 * @brief 获取连接显示名称
 * @return "TCP Server:端口" 格式字符串
 */
QString TcpServerConnection::name() const
{
    if (m_listening) {
        return QString("TCP Server:%1").arg(m_listenPort);
    }
    return tr("TCP Server (未监听)");
}

/**
 * @brief 获取当前连接状态
 * @return 连接状态枚举
 */
ConnectionState TcpServerConnection::state() const
{
    return m_state;
}

/**
 * @brief 打开连接 - 开始监听
 * @return true=监听成功，false=失败
 */
bool TcpServerConnection::open()
{
    return listen(m_listenAddress, m_listenPort);
}

/**
 * @brief 关闭连接 - 停止监听并断开所有客户端
 */
void TcpServerConnection::close()
{
    stopListening();
}

/**
 * @brief 发送数据 - 广播到所有已连接客户端
 * @param data 待发送的字节数据
 * @return 成功发送的客户端数量
 */
qint64 TcpServerConnection::write(const QByteArray& data)
{
    return broadcastToClients(data);
}

/**
 * @brief 配置TCP服务器参数
 * @param params 参数映射:
 *   - "listenAddress": QString (监听地址)
 *   - "port": int (监听端口号)
 */
void TcpServerConnection::configure(const QVariantMap& params)
{
    if (params.contains("listenAddress")) {
        m_listenAddress = QHostAddress(params["listenAddress"].toString());
    }
    if (params.contains("port")) {
        m_listenPort = static_cast<quint16>(params["port"].toInt());
    }
}

/**
 * @brief 开始监听指定地址和端口
 * @param address 监听地址
 * @param port 监听端口号
 * @return true=监听成功
 */
bool TcpServerConnection::listen(const QHostAddress& address, int port)
{
    Q_UNUSED(address)
    Q_UNUSED(port)
    // TODO: 实现QTcpServer监听逻辑
    m_listening = true;
    updateState(ConnectionState::Connected);
    return true;
}

/**
 * @brief 停止监听 - 断开所有客户端并关闭服务器
 */
void TcpServerConnection::stopListening()
{
    // TODO: 断开所有客户端，关闭服务器
    for (auto* socket : m_clientSockets) {
        socket->disconnectFromHost();
    }
    m_clientSockets.clear();
    m_clients.clear();
    m_listening = false;
    updateState(ConnectionState::Disconnected);
}

/**
 * @brief 获取已连接的客户端列表
 * @return 客户端地址:端口字符串列表
 */
QStringList TcpServerConnection::connectedClients() const
{
    return m_clients;
}

/**
 * @brief 向所有已连接客户端广播数据
 * @param data 待广播的字节数据
 * @return 成功发送的客户端数量
 */
int TcpServerConnection::broadcastToClients(const QByteArray& data)
{
    Q_UNUSED(data)
    // TODO: 遍历所有客户端socket发送数据
    int count = 0;
    for (auto* socket : m_clientSockets) {
        if (socket->state() == QAbstractSocket::ConnectedState) {
            socket->write(data);
            count++;
        }
    }
    return count;
}

/**
 * @brief 新客户端连接回调
 */
void TcpServerConnection::onNewConnection()
{
    // TODO: 接受新连接，加入客户端列表
}

/**
 * @brief 客户端断开回调
 */
void TcpServerConnection::onClientDisconnected()
{
    // TODO: 从客户端列表移除
}

/**
 * @brief 客户端数据到达回调
 */
void TcpServerConnection::onClientReadyRead()
{
    // TODO: 读取客户端数据并发射信号
}

/**
 * @brief 更新连接状态
 * @param newState 新状态
 */
void TcpServerConnection::updateState(ConnectionState newState)
{
    if (m_state != newState) {
        m_state = newState;
        emit stateChanged(newState);
    }
}

/**
 * @brief 获取socket对应的客户端标识
 * @param socket 客户端socket
 * @return "地址:端口" 格式字符串
 */
QString TcpServerConnection::clientInfo(QTcpSocket* socket)
{
    if (!socket) return QString();
    return QString("%1:%2").arg(socket->peerAddress().toString()).arg(socket->peerPort());
}
