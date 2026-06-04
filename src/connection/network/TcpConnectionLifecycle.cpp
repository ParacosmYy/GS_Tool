/**
 * @file TcpConnectionLifecycle.cpp
 * @brief TCP连接生命周期方法 — 打开/关闭/写入/配置/析构
 *
 * 从 TcpConnection.cpp 拆分而来，包含TCP连接的核心生命周期方法:
 *   - open(): 客户端连接/服务端监听
 *   - close(): 断开连接并释放资源
 *   - write(): 数据发送
 *   - configure(): 参数配置
 *   - 构造/析构
 *
 * Socket事件处理器见 TcpConnectionHandlers.cpp。
 * 统计/辅助方法见 TcpConnectionStats.cpp / TcpConnectionHelpers.cpp。
 */

#include "connection/network/TcpConnection.h"
#include "shared/ConnectionConstants.h"
#include "shared/TimerConstants.h"
#include <QNetworkInterface>

/** @brief 构造TCP连接，初始化内部socket/server/timer为空 @param parent 父对象 */
TcpConnection::TcpConnection(QObject* parent)
    : IConnection(parent)
{
}

/** @brief 析构函数，静默关闭socket/server/timer(不发射stateChanged信号，避免析构期间回调) */
TcpConnection::~TcpConnection()
{
    // 析构时仅释放资源，不发射信号(避免析构期间回调)
    if (m_clientSocket) {
        m_clientSocket->disconnectFromHost();
        m_clientSocket = nullptr;
    }
    if (m_server) {
        m_server->close();
        m_server = nullptr;
    }
    if (m_connectTimer) {
        m_connectTimer->stop();
        m_connectTimer = nullptr;
    }
    m_state = ConnectionState::Disconnected;
}

/** @brief 返回连接类型(TCP客户端或TCP服务端) @return ConnectionType枚举值 */
ConnectionType TcpConnection::type() const
{
    return (m_mode == Server) ? ConnectionType::TcpServer : ConnectionType::TcpClient;
}

/** @brief 返回连接名称(格式: TCP:host:port 或 TCP Server:port) @return 连接名称字符串 */
QString TcpConnection::name() const
{
    if (m_mode == Client) {
        return QString("TCP:%1:%2").arg(m_host).arg(m_port);
    }
    return QString("TCP Server:%1").arg(m_port);
}

/** @brief 返回当前连接状态 @return ConnectionState枚举值 */
ConnectionState TcpConnection::state() const
{
    return m_state;
}

/** @brief 从参数映射配置连接参数 @param params 参数映射，支持"host"/"port"/"mode"键 */
void TcpConnection::configure(const QVariantMap& params)
{
    m_host = params.value("host", ConnectionDefaults::kDefaultHost).toString();
    m_port = static_cast<quint16>(params.value("port", ConnectionDefaults::kDefaultPort).toInt());
    m_mode = params.value("mode", "client").toString() == "server" ? Server : Client;
}

/** @brief 打开TCP连接，客户端模式连接远端并启动10秒超时定时器，服务端模式监听端口 @return true表示成功发起连接或开始监听 */
bool TcpConnection::open()
{
    ++m_totalOpenAttempts;
    if (m_mode == Client) {
        if (!m_socket) {
            m_socket = new QTcpSocket(this);
            m_isReconnectAttempt = false;  // 首次连接，清除重连标记
            connect(m_socket, &QTcpSocket::connected,
                    this, &TcpConnection::onSocketConnected);
            connect(m_socket, &QTcpSocket::disconnected,
                    this, &TcpConnection::onSocketDisconnected);
            connect(m_socket, &QTcpSocket::readyRead,
                    this, &TcpConnection::onSocketReadyRead);
            connect(m_socket, &QTcpSocket::errorOccurred,
                    this, &TcpConnection::onSocketError);
            // 转发底层写入完成信号，供上层OTA进度追踪和发送统计
            connect(m_socket, &QTcpSocket::bytesWritten,
                    this, &TcpConnection::bytesWritten);
        } else if (m_socket->state() != QAbstractSocket::UnconnectedState) {
            // 已有socket且非断开状态 — 视为重连尝试
            ++m_totalReconnectAttempts;
            m_isReconnectAttempt = true;  // 标记为重连，onSocketConnected中用于计数
            m_socket->abort();  // 中断当前连接，准备重连
        }

        updateState(ConnectionState::Connecting);
        m_connectStartTime.start();  // 记录连接发起时刻，用于延迟计算
        ++m_totalDnsLookups;  // 每次connectToHost触发一次DNS查询
        m_socket->connectToHost(m_host, m_port);
        // 启用TCP KeepAlive，长连接场景下可及时检测对端断开
        m_socket->setSocketOption(QAbstractSocket::KeepAliveOption, 1);

        // 启动10秒连接超时定时器，防止连接不可达主机时无限等待
        if (!m_connectTimer) {
            m_connectTimer = new QTimer(this);
            m_connectTimer->setSingleShot(true);
            connect(m_connectTimer, &QTimer::timeout, this, [this]() {
                if (m_socket && m_socket->state() == QAbstractSocket::ConnectingState) {
                    m_socket->abort();
                    ++m_errorCount;  // 连接超时计为错误
                    ++m_totalConnectionTimeouts;  // 连接超时计数
                    emit errorOccurred(tr("连接超时，请检查目标主机是否可达"));
                    updateState(ConnectionState::Error);
                }
            });
        }
        m_connectTimer->start(Timers::kConnectTimeoutMs);

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
            ++m_errorCount;  // 监听失败计为错误
            emit errorOccurred(tr("TCP服务器监听失败: %1").arg(m_server->errorString()));
            updateState(ConnectionState::Error);
            return false;
        }

        updateState(ConnectionState::Connected);
        ++m_totalConnections;  // 服务端监听成功计为一次连接
        return true;
    }
}

/** @brief 关闭TCP连接，停止超时定时器，断开信号连接并释放socket/server资源，仅已连接状态下计数 */
void TcpConnection::close()
{
    // 停止连接超时定时器
    if (m_connectTimer) m_connectTimer->stop();

    // 统计断开次数: 仅在已连接状态下关闭时计数
    bool wasConnected = (m_state == ConnectionState::Connected);

    if (m_socket) {
        disconnect(m_socket, nullptr, this, nullptr);  // 防止信号在 deleteLater 之前到达
        m_socket->disconnectFromHost();
        m_socket->deleteLater();
        m_socket = nullptr;
    }
    if (m_clientSocket) {
        disconnect(m_clientSocket, nullptr, this, nullptr);  // 防止信号在 deleteLater 之前到达
        m_clientSocket->disconnectFromHost();
        m_clientSocket->deleteLater();
        m_clientSocket = nullptr;
    }
    if (m_server) {
        m_server->close();
        m_server->deleteLater();
        m_server = nullptr;
    }

    if (wasConnected) {
        ++m_totalDisconnections;  // 从已连接状态断开时计数
    }
    updateState(ConnectionState::Disconnected);
}

/** @brief 写入数据到TCP连接(客户端写m_socket，服务端写m_clientSocket) @param data 待发送的字节数据 @return 实际写入字节数，-1表示失败 */
qint64 TcpConnection::write(const QByteArray& data)
{
    ++m_totalWrites;
    QTcpSocket* target = nullptr;
    if (m_mode == Client) {
        target = m_socket;
    } else {
        target = m_clientSocket;
    }

    if (!target || target->state() != QAbstractSocket::ConnectedState) {
        return -1;
    }

    qint64 written = target->write(data);
    if (written < 0) {
        ++m_errorCount;  // 写入失败计为错误
        emit errorOccurred(tr("TCP写入失败: %1").arg(target->errorString()));
    } else {
        m_totalBytesSent += static_cast<quint64>(written);  // 累计发送字节
    }
    return written;
}
