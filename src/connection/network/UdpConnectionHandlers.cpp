/**
 * @file UdpConnectionHandlers.cpp
 * @brief UDP连接 — 错误处理与状态更新方法
 *
 * 从 UdpConnection.cpp 拆分而来，包含:
 *   - onReadyRead(): 数据报接收处理
 *   - onError(): socket错误处理
 *   - translateNetworkError(): 网络错误码翻译
 *   - updateState(): 连接状态更新
 *
 * 连接生命周期(open/close/write/configure)保留在 UdpConnection.cpp。
 * 多播组管理/统计getter/resetStats见 UdpConnectionStats.cpp。
 */

#include "connection/network/UdpConnection.h"

/** @brief readyRead信号处理，读取所有到达的数据报并转发给上层，同时累计接收字节和数据报统计 */
void UdpConnection::onReadyRead()
{
    if (!m_socket) return;

    while (m_socket->hasPendingDatagrams()) {
        QByteArray buffer;
        buffer.resize(static_cast<int>(m_socket->pendingDatagramSize()));
        QHostAddress senderAddr;
        quint16 senderPort;
        qint64 bytesRead = m_socket->readDatagram(buffer.data(), buffer.size(),
                                                   &senderAddr, &senderPort);
        if (bytesRead < 0) {
            ++m_totalDatagramErrors;  // 数据报接收失败计数
            continue;
        }
        buffer.resize(static_cast<int>(bytesRead));
        ++m_totalDatagramsReceived;
        m_totalBytesReceived += static_cast<quint64>(bytesRead);
        if (!buffer.isEmpty()) {
            emit dataReceived(buffer);
        }
    }
}

/** @brief socket错误处理，递增错误计数并翻译错误码为中文描述后通知上层 @param error Qt网络错误码 */
void UdpConnection::onError(QAbstractSocket::SocketError error)
{
    ++m_errorCount;
    ++m_totalSocketErrors;  ///< 累计Socket错误次数
    QString systemError = m_socket ? m_socket->errorString() : QString();
    emit errorOccurred(translateNetworkError(error, systemError));
    updateState(ConnectionState::Error);
}

/** @brief 将网络错误码翻译为中文描述 @param error Qt网络错误枚举 @param systemError 系统级错误描述字符串 @return 人类可读的中文错误描述 */
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

/** @brief 更新连接状态(仅当状态变化时发射stateChanged信号) @param newState 新的连接状态 */
void UdpConnection::updateState(ConnectionState newState)
{
    if (m_state != newState) {
        m_state = newState;
        emit stateChanged(newState);
    }
}
