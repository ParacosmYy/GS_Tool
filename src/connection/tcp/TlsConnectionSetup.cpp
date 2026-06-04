/**
 * @file TlsConnectionSetup.cpp
 * @brief TLS连接 - 证书设置与SSL回调实现
 *
 * 从 TlsConnection.cpp 拆分而来，包含证书/CA/对端验证配置
 * 和SSL回调处理(加密完成/SSL错误/数据到达/状态变更)。
 */

#include "connection/tcp/TlsConnection.h"
#include <QSslConfiguration>
#include <QFile>

/** @brief 设置本地证书和私钥文件路径 @param certPath 证书文件路径(PEM格式) @param keyPath 私钥文件路径(PEM格式) */
void TlsConnection::setCertificate(const QString& certPath, const QString& keyPath)
{
    m_certPath = certPath;
    m_keyPath = keyPath;
}

/** @brief 设置CA证书文件路径用于验证对端 @param caPath CA证书文件路径 */
void TlsConnection::setCaCertificate(const QString& caPath)
{
    m_caPath = caPath;
}

/** @brief 设置是否验证对端证书 @param verify true=验证对端，false=不验证 */
void TlsConnection::setPeerVerify(bool verify)
{
    m_peerVerify = verify;
}

/** @brief SSL加密通道建立完成回调，更新状态为Connected */
void TlsConnection::onEncrypted()
{
    ++m_totalHandshakes;
    updateState(ConnectionState::Connected);
}

/** @brief SSL错误处理，不验证对端时忽略错误，否则累计错误计数 @param errors SSL错误列表 */
void TlsConnection::onSslErrors(const QList<QSslError>& errors)
{
    ++m_totalSslErrors;
    if (!m_peerVerify) {
        /// 不验证对端时，忽略所有SSL错误继续连接
        m_socket->ignoreSslErrors(errors);
        return;
    }

    /// 验证模式下报告错误
    ++m_errorCount;
    QStringList errorStrs;
    for (const QSslError& e : errors) {
        errorStrs.append(e.errorString());
    }
    emit errorOccurred(tr("TLS证书验证失败: %1").arg(errorStrs.join("; ")));
}

/** @brief 数据到达回调，累计接收字节数并发射dataReceived信号 */
void TlsConnection::onReadyRead()
{
    if (!m_socket) return;

    QByteArray data = m_socket->readAll();
    if (!data.isEmpty()) {
        m_totalBytesReceived += static_cast<quint64>(data.size());
        emit dataReceived(data);
    }
}

/** @brief socket连接状态变化回调，UnconnectedState时更新为Disconnected @param socketState 当前socket状态 */
void TlsConnection::onStateChanged(QAbstractSocket::SocketState socketState)
{
    if (socketState == QAbstractSocket::UnconnectedState) {
        updateState(ConnectionState::Disconnected);
    }
}
