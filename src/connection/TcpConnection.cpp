/**
 * @file TcpConnection.cpp
 * @brief TCP连接实现 - 封装 QTcpSocket 的可靠流式通信
 *
 * 支持客户端模式连接远程TCP服务器，包含:
 *   - 10秒连接超时保护（防止无限等待）
 *   - 自动重连时的正确清理
 *   - 详细的错误翻译和中文诊断信息
 */

#include "connection/TcpConnection.h"
#include "core/Constants.h"
#include <QNetworkInterface>

/** @brief 构造TCP连接(初始化QTcpSocket) @param parent 父对象 */
TcpConnection::TcpConnection(QObject* parent)
    : IConnection(parent)
{
}

TcpConnection::~TcpConnection()
{
    close();
}

/** @brief 返回连接类型(TCP客户端或TCP服务端) @return ConnectionType枚举 */
ConnectionType TcpConnection::type() const
{
    return (m_mode == Server) ? ConnectionType::TcpServer : ConnectionType::TcpClient;
}

/** @brief 返回连接名称(格式: TCP:host:port 或 TCP:server:port) @return 连接名称 */
QString TcpConnection::name() const
{
    if (m_mode == Client) {
        return QString("TCP:%1:%2").arg(m_host).arg(m_port);
    }
    return QString("TCP Server:%1").arg(m_port);
}

/** @brief 返回当前连接状态 @return ConnectionState枚举 */
ConnectionState TcpConnection::state() const
{
    return m_state;
}

/** @brief 从参数映射配置连接(host/port/mode) @param params 参数映射，支持"host"/"port"/"mode"键 */
void TcpConnection::configure(const QVariantMap& params)
{
    m_host = params.value("host", "127.0.0.1").toString();
    m_port = static_cast<quint16>(params.value("port", 8080).toInt());
    m_mode = params.value("mode", "client").toString() == "server" ? Server : Client;
}

/** @brief 打开TCP连接(客户端模式连接远端，服务端模式监听端口) @return true表示成功发起连接或开始监听 */
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
            // 转发底层写入完成信号，供上层OTA进度追踪和发送统计
            connect(m_socket, &QTcpSocket::bytesWritten,
                    this, &TcpConnection::bytesWritten);
        }

        updateState(ConnectionState::Connecting);
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
            emit errorOccurred(tr("TCP服务器监听失败: %1").arg(m_server->errorString()));
            updateState(ConnectionState::Error);
            return false;
        }

        updateState(ConnectionState::Connected);
        return true;
    }
}

/** @brief 关闭TCP连接(释放socket/server/timer资源) */
void TcpConnection::close()
{
    // 停止连接超时定时器
    if (m_connectTimer) m_connectTimer->stop();

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
    updateState(ConnectionState::Disconnected);
}

/** @brief 写入数据到TCP连接(客户端写m_socket，服务端写m_clientSocket) @param data 待发送数据 @return 实际写入字节数，-1表示失败 */
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

    qint64 written = target->write(data);
    if (written < 0) {
        emit errorOccurred(tr("TCP写入失败: %1").arg(target->errorString()));
    }
    return written;
}

/** @brief 客户端模式：socket连接成功回调，停止超时定时器并更新状态 */
void TcpConnection::onSocketConnected()
{
    // 连接成功，取消超时定时器
    if (m_connectTimer) m_connectTimer->stop();
    updateState(ConnectionState::Connected);
}

/** @brief 客户端模式：socket断开回调，更新状态为Disconnected */
void TcpConnection::onSocketDisconnected()
{
    updateState(ConnectionState::Disconnected);
}

/** @brief socket可读回调，读取全部数据并发射dataReceived信号 */
void TcpConnection::onSocketReadyRead()
{
    QTcpSocket* senderSock = qobject_cast<QTcpSocket*>(sender());
    if (!senderSock) return;

    QByteArray data = senderSock->readAll();
    if (!data.isEmpty()) {
        emit dataReceived(data);
    }
}

/** @brief socket错误回调，翻译错误码并发射errorOccurred信号 @param error Qt网络错误枚举 */
void TcpConnection::onSocketError(QAbstractSocket::SocketError error)
{
    QTcpSocket* sock = qobject_cast<QTcpSocket*>(sender());
    QString systemError = sock ? sock->errorString() : QString();
    emit errorOccurred(translateNetworkError(error, systemError));
    updateState(ConnectionState::Error);
}

/** @brief 将Qt网络错误码翻译为用户友好的中文诊断信息 @param error Qt网络错误枚举 @param systemError 系统错误字符串 @return 中文错误描述 */
QString TcpConnection::translateNetworkError(QAbstractSocket::SocketError error,
                                              const QString& systemError)
{
    switch (error) {
    case QAbstractSocket::ConnectionRefusedError:
        return TcpConnection::tr("连接被拒绝，请检查目标地址和端口是否正确");
    case QAbstractSocket::RemoteHostClosedError:
        return TcpConnection::tr("远程主机已关闭连接");
    case QAbstractSocket::HostNotFoundError:
        return TcpConnection::tr("无法解析主机名，请检查地址是否正确");
    case QAbstractSocket::NetworkError:
        return TcpConnection::tr("网络异常，请检查网络连接");
    case QAbstractSocket::SocketAccessError:
        return TcpConnection::tr("套接字访问被拒绝，权限不足");
    case QAbstractSocket::SocketResourceError:
        return TcpConnection::tr("系统资源不足，无法创建套接字");
    case QAbstractSocket::SocketTimeoutError:
        return TcpConnection::tr("连接超时，请检查目标主机是否可达");
    case QAbstractSocket::DatagramTooLargeError:
        return TcpConnection::tr("数据报过大，超出系统限制");
    case QAbstractSocket::AddressInUseError:
        return TcpConnection::tr("地址/端口已被占用，请更换端口");
    case QAbstractSocket::SocketAddressNotAvailableError:
        return TcpConnection::tr("请求的地址不可用");
    case QAbstractSocket::UnsupportedSocketOperationError:
        return TcpConnection::tr("不支持的操作");
    case QAbstractSocket::ProxyAuthenticationRequiredError:
        return TcpConnection::tr("代理服务器需要认证");
    case QAbstractSocket::SslHandshakeFailedError:
        return TcpConnection::tr("SSL/TLS握手失败");
    case QAbstractSocket::SslInternalError:
        return TcpConnection::tr("SSL/TLS内部错误");
    case QAbstractSocket::SslInvalidUserDataError:
        return TcpConnection::tr("SSL/TLS证书数据无效");
    case QAbstractSocket::TemporaryError:
        return TcpConnection::tr("临时错误，请稍后重试");
    default:
        break;
    }
    if (systemError.isEmpty())
        return TcpConnection::tr("未知TCP错误");
    return TcpConnection::tr("TCP错误: %1").arg(systemError);
}

/** @brief 服务端模式：新客户端连接回调，替换旧客户端并连接信号 */
void TcpConnection::onNewConnection()
{
    if (m_clientSocket) {
        // 断开旧客户端的所有信号连接，防止 disconnected lambda 在新客户端赋值后删除错误的 socket
        disconnect(m_clientSocket, nullptr, this, nullptr);
        m_clientSocket->disconnectFromHost();
        m_clientSocket->deleteLater();
    }

    m_clientSocket = m_server->nextPendingConnection();
    if (m_clientSocket) {
        QTcpSocket* sock = m_clientSocket;  // 捕获当前 socket 指针，防止 lambda 通过 m_clientSocket 访问到新 socket
        connect(sock, &QTcpSocket::readyRead,
                this, &TcpConnection::onSocketReadyRead);
        connect(sock, &QTcpSocket::disconnected,
                this, [this, sock]() {
                    // 仅当 m_clientSocket 仍指向本 socket 时才清理（新连接已替换则跳过）
                    if (m_clientSocket == sock) {
                        m_clientSocket->deleteLater();
                        m_clientSocket = nullptr;
                    }
                });
        connect(sock, &QTcpSocket::errorOccurred,
                this, &TcpConnection::onSocketError);
        updateState(ConnectionState::Connected);
    }
}

/** @brief 更新连接状态并发射stateChanged信号(仅当状态真正变化时) @param newState 新状态 */
void TcpConnection::updateState(ConnectionState newState)
{
    if (m_state != newState) {
        m_state = newState;
        emit stateChanged(newState);
    }
}
