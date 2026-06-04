/**
 * @file TcpConnectionHelpers.cpp
 * @brief TCP连接辅助方法实现 - 错误翻译与状态管理
 *
 * 从 TcpConnection.cpp 拆分而来，包含:
 *   - translateNetworkError: Qt网络错误码翻译为中文诊断信息
 *   - updateState: 连接状态更新与信号发射
 */

#include "connection/network/TcpConnection.h"

// ---- 连接生命周期辅助方法 ----

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

/** @brief 更新连接状态并发射stateChanged信号(仅当状态真正变化时才发射) @param newState 新的连接状态 */
void TcpConnection::updateState(ConnectionState newState)
{
    if (m_state != newState) {
        m_state = newState;
        emit stateChanged(newState);
    }
}
