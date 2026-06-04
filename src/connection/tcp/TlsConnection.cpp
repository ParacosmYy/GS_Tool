/**
 * @file TlsConnection.cpp
 * @brief TLS/SSL安全连接实现
 */

#include "connection/tcp/TlsConnection.h"
#include <QSslConfiguration>
#include <QFile>

/** @brief 构造TLS连接对象 @param parent 父QObject指针 */
TlsConnection::TlsConnection(QObject* parent)
    : IConnection(parent)
{
}

/** @brief 析构TLS连接，关闭并释放SSL socket资源 */
TlsConnection::~TlsConnection()
{
    close();
}

/** @brief 获取连接类型 @return ConnectionType::Tls */
ConnectionType TlsConnection::type() const
{
    return ConnectionType::Tls;
}

/** @brief 获取连接显示名称 @return 已连接时返回"TLS://主机:端口"格式，否则返回"未连接" */
QString TlsConnection::name() const
{
    if (m_state == ConnectionState::Connected) {
        return tr("TLS://%1:%2").arg(m_host).arg(m_port);
    }
    return tr("TLS (未连接)");
}

/** @brief 获取当前连接状态 @return 连接状态枚举值 */
ConnectionState TlsConnection::state() const
{
    return m_state;
}

/** @brief 打开TLS连接，加载证书/私钥/CA并发起加密握手 @return true=握手已启动，false=配置错误 */
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
                ++m_errorCount;
                emit errorOccurred(m_socket->errorString());
                updateState(ConnectionState::Error);
            });

    updateState(ConnectionState::Connecting);
    ++m_totalOpens;
    m_socket->connectToHostEncrypted(m_host, m_port);
    return true;
}

/** @brief 关闭TLS连接，断开信号并释放SSL socket */
void TlsConnection::close()
{
    ++m_totalCloses;
    if (m_socket) {
        disconnect(m_socket, nullptr, this, nullptr);  // 断开所有信号防止析构期间回调
        m_socket->disconnectFromHost();
        m_socket->deleteLater();
        m_socket = nullptr;
    }
    updateState(ConnectionState::Disconnected);
}

/** @brief 发送加密数据 @param data 待发送数据 @return 发送字节数，未连接返回-1 */
qint64 TlsConnection::write(const QByteArray& data)
{
    ++m_totalWrites;
    if (!m_socket || m_state != ConnectionState::Connected) {
        return -1;
    }
    qint64 written = m_socket->write(data);
    if (written > 0) {
        m_totalBytesSent += static_cast<quint64>(written);
        m_socket->flush();
        emit bytesWritten(written);
    } else if (written < 0) {
        ++m_errorCount;
    }
    return written;
}

/** @brief 配置TLS连接参数(host/port/certPath/keyPath/caPath/peerVerify) @param params 参数映射 */
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

/** @brief 更新连接状态，状态变化时发射stateChanged信号 @param newState 新状态 */
void TlsConnection::updateState(ConnectionState newState)
{
    if (m_state != newState) {
        m_state = newState;
        emit stateChanged(newState);
    }
}

// 证书设置/SSL回调见 TlsConnectionSetup.cpp
// 统计getter/resetStats见 TlsConnectionStats.cpp
