/**
 * @file TlsConnection.cpp
 * @brief TLS/SSL安全连接实现
 */

#include "connection/tcp/TlsConnection.h"
#include <QSslConfiguration>
#include <QFile>

/**
 * @brief 构造函数
 * @param parent 父对象
 */
TlsConnection::TlsConnection(QObject* parent)
    : IConnection(parent)
{
}

/**
 * @brief 析构函数 - 关闭连接
 */
TlsConnection::~TlsConnection()
{
    close();
}

/**
 * @brief 获取连接类型
 * @return TCP客户端类型(通过TLS封装)
 */
ConnectionType TlsConnection::type() const
{
    return ConnectionType::Tls;
}

/**
 * @brief 获取连接显示名称
 * @return "TLS://主机:端口" 格式
 */
QString TlsConnection::name() const
{
    if (m_state == ConnectionState::Connected) {
        return tr("TLS://%1:%2").arg(m_host).arg(m_port);
    }
    return tr("TLS (未连接)");
}

/**
 * @brief 获取当前状态
 */
ConnectionState TlsConnection::state() const
{
    return m_state;
}

/**
 * @brief 打开TLS连接 - 发起加密握手
 * @return true=握手已启动
 */
bool TlsConnection::open()
{
    if (m_state == ConnectionState::Connected) {
        return true;
    }

    /* 防止重复open()导致内存泄漏 — 清理旧socket */
    if (m_socket) {
        m_socket->deleteLater();
        m_socket = nullptr;
    }

    m_socket = new QSslSocket(this);

    /// 加载本地证书和私钥
    if (!m_certPath.isEmpty() && !m_keyPath.isEmpty()) {
        QFile certFile(m_certPath);
        if (!certFile.open(QIODevice::ReadOnly)) {
            emit errorOccurred(tr("无法打开证书文件: %1").arg(m_certPath));
            return false;
        }
        QSslCertificate cert(&certFile, QSsl::Pem);
        if (cert.isNull()) {
            emit errorOccurred(tr("证书解析失败: %1").arg(m_certPath));
            return false;
        }
        m_socket->setLocalCertificate(cert);
        certFile.close();

        QFile keyFile(m_keyPath);
        if (!keyFile.open(QIODevice::ReadOnly)) {
            emit errorOccurred(tr("无法打开私钥文件: %1").arg(m_keyPath));
            return false;
        }
        QSslKey key(&keyFile, QSsl::Rsa, QSsl::Pem);
        if (key.isNull()) {
            emit errorOccurred(tr("私钥解析失败: %1").arg(m_keyPath));
            return false;
        }
        m_socket->setPrivateKey(key);
        keyFile.close();
    }

    /// 加载CA证书
    if (!m_caPath.isEmpty()) {
        QFile caFile(m_caPath);
        if (!caFile.open(QIODevice::ReadOnly)) {
            emit errorOccurred(tr("无法打开CA证书文件: %1").arg(m_caPath));
            return false;
        }
        QSslCertificate caCert(&caFile, QSsl::Pem);
        if (caCert.isNull()) {
            emit errorOccurred(tr("CA证书解析失败: %1").arg(m_caPath));
            return false;
        }
        QSslConfiguration sslConfig = m_socket->sslConfiguration();
        sslConfig.addCaCertificate(caCert);
        m_socket->setSslConfiguration(sslConfig);
        caFile.close();
    }

    /// 设置对端验证模式
    if (m_peerVerify) {
        m_socket->setPeerVerifyMode(QSslSocket::VerifyPeer);
    } else {
        m_socket->setPeerVerifyMode(QSslSocket::QueryPeer);
    }

    /// 连接信号
    connect(m_socket, &QSslSocket::encrypted,
            this, &TlsConnection::onEncrypted);
    connect(m_socket, &QSslSocket::sslErrors,
            this, &TlsConnection::onSslErrors);
    connect(m_socket, &QSslSocket::readyRead,
            this, &TlsConnection::onReadyRead);
    connect(m_socket, &QSslSocket::stateChanged,
            this, &TlsConnection::onStateChanged);
    connect(m_socket, &QAbstractSocket::errorOccurred,
            this, [this](QAbstractSocket::SocketError err) {
                Q_UNUSED(err)
                emit errorOccurred(m_socket->errorString());
                updateState(ConnectionState::Error);
            });

    updateState(ConnectionState::Connecting);
    m_socket->connectToHostEncrypted(m_host, m_port);
    return true;
}

/**
 * @brief 关闭TLS连接
 */
void TlsConnection::close()
{
    if (m_socket) {
        disconnect(m_socket, nullptr, this, nullptr);  // 断开所有信号防止析构期间回调
        m_socket->disconnectFromHost();
        m_socket->deleteLater();
        m_socket = nullptr;
    }
    updateState(ConnectionState::Disconnected);
}

/**
 * @brief 发送加密数据
 * @param data 待发送数据
 * @return 发送字节数
 */
qint64 TlsConnection::write(const QByteArray& data)
{
    if (!m_socket || m_state != ConnectionState::Connected) {
        return -1;
    }
    qint64 written = m_socket->write(data);
    if (written > 0) {
        m_txBytes += written;
        m_socket->flush();
        emit bytesWritten(written);
    }
    return written;
}

/**
 * @brief 配置TLS连接参数
 * @param params 参数映射
 */
void TlsConnection::configure(const QVariantMap& params)
{
    if (params.contains("host")) {
        m_host = params["host"].toString();
    }
    if (params.contains("port")) {
        m_port = static_cast<quint16>(params["port"].toInt());
    }
    if (params.contains("certPath")) {
        m_certPath = params["certPath"].toString();
    }
    if (params.contains("keyPath")) {
        m_keyPath = params["keyPath"].toString();
    }
    if (params.contains("caPath")) {
        m_caPath = params["caPath"].toString();
    }
    if (params.contains("peerVerify")) {
        m_peerVerify = params["peerVerify"].toBool();
    }
}

/**
 * @brief 设置本地证书和私钥
 */
void TlsConnection::setCertificate(const QString& certPath, const QString& keyPath)
{
    m_certPath = certPath;
    m_keyPath = keyPath;
}

/**
 * @brief 设置CA证书
 */
void TlsConnection::setCaCertificate(const QString& caPath)
{
    m_caPath = caPath;
}

/**
 * @brief 设置是否验证对端证书
 */
void TlsConnection::setPeerVerify(bool verify)
{
    m_peerVerify = verify;
}

/**
 * @brief SSL加密通道建立完成
 */
void TlsConnection::onEncrypted()
{
    ++m_handshakeCount;
    updateState(ConnectionState::Connected);
}

/**
 * @brief SSL错误处理
 * @param errors SSL错误列表
 */
void TlsConnection::onSslErrors(const QList<QSslError>& errors)
{
    if (!m_peerVerify) {
        /// 不验证对端时，忽略所有SSL错误继续连接
        m_socket->ignoreSslErrors(errors);
        return;
    }

    /// 验证模式下报告错误
    QStringList errorStrs;
    for (const QSslError& e : errors) {
        errorStrs.append(e.errorString());
    }
    emit errorOccurred(tr("TLS证书验证失败: %1").arg(errorStrs.join("; ")));
}

/**
 * @brief 数据到达回调
 */
void TlsConnection::onReadyRead()
{
    if (!m_socket) return;

    QByteArray data = m_socket->readAll();
    if (!data.isEmpty()) {
        m_rxBytes += data.size();
        emit dataReceived(data);
    }
}

/**
 * @brief socket连接状态变化回调
 * @param socketState 当前socket状态
 */
void TlsConnection::onStateChanged(QAbstractSocket::SocketState socketState)
{
    if (socketState == QAbstractSocket::UnconnectedState) {
        updateState(ConnectionState::Disconnected);
    }
}

/**
 * @brief 更新连接状态
 */
void TlsConnection::updateState(ConnectionState newState)
{
    if (m_state != newState) {
        m_state = newState;
        emit stateChanged(newState);
    }
}

/**
 * @brief 获取已发送字节数
 */
qint64 TlsConnection::totalBytesSent() const
{
    return m_txBytes;
}

/**
 * @brief 获取已接收字节数
 */
qint64 TlsConnection::totalBytesReceived() const
{
    return m_rxBytes;
}

/**
 * @brief 获取SSL握手次数
 */
quint64 TlsConnection::handshakeCount() const
{
    return m_handshakeCount;
}

/**
 * @brief 重置统计数据
 */
void TlsConnection::resetStatistics()
{
    m_txBytes = 0;
    m_rxBytes = 0;
    m_handshakeCount = 0;
}
