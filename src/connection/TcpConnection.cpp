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
    return (m_mode == Server) ? ConnectionType::TcpServer : ConnectionType::TcpClient;
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
        m_connectTimer->start(10000);

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
    // 连接成功，取消超时定时器
    if (m_connectTimer) m_connectTimer->stop();
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
    QTcpSocket* sock = qobject_cast<QTcpSocket*>(sender());
    QString systemError = sock ? sock->errorString() : QString();
    emit errorOccurred(translateNetworkError(error, systemError));
    updateState(ConnectionState::Error);
}

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

void TcpConnection::updateState(ConnectionState newState)
{
    if (m_state != newState) {
        m_state = newState;
        emit stateChanged(newState);
    }
}
