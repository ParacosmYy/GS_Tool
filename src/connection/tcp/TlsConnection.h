/**
 * @file TlsConnection.h
 * @brief TLS/SSL安全连接 - 在TCP基础上封装SSL/TLS加密通道
 *
 * 职责:
 *   1. 提供SSL/TLS加密的TCP连接
 *   2. 支持证书配置和客户端/服务端验证
 *   3. 复用IConnection抽象接口
 *
 * 协作关系:
 *   - ConnectionFactory: 通过工厂创建实例
 *   - ConnectionController: 管理连接生命周期
 */

#ifndef TLSCONNECTION_H
#define TLSCONNECTION_H

#include "connection/interface/IConnection.h"
#include <QSslSocket>
#include <QSslCertificate>
#include <QSslKey>

/**
 * @brief TLS/SSL安全连接实现
 *
 * 在标准TCP连接基础上增加SSL/TLS加密层，
 * 适用于需要安全通信的嵌入式设备管理场景。
 */
class TlsConnection : public IConnection {
    Q_OBJECT

public:
    /** @brief 构造函数
     * @param parent 父对象
     */
    explicit TlsConnection(QObject* parent = nullptr);

    /** @brief 析构，关闭连接 */
    ~TlsConnection() override;

    // ---- IConnection接口实现 ----
    ConnectionType type() const override;              ///< 返回连接类型(TLS)
    QString name() const override;                     ///< 返回连接显示名称
    ConnectionState state() const override;            ///< 返回当前连接状态
    bool open() override;                              ///< 发起SSL/TLS连接
    void close() override;                             ///< 关闭SSL/TLS连接
    qint64 write(const QByteArray& data) override;     ///< 通过加密通道发送数据
    void configure(const QVariantMap& params) override; ///< 配置TLS参数(host/port/cert/key/ca)

    // ---- TLS特有接口 ----

    /**
     * @brief 设置本地证书和私钥
     * @param certPath 证书文件路径(PEM格式)
     * @param keyPath 私钥文件路径(PEM格式)
     */
    void setCertificate(const QString& certPath, const QString& keyPath);

    /**
     * @brief 设置CA证书用于验证对端
     * @param caPath CA证书文件路径
     */
    void setCaCertificate(const QString& caPath);

    /**
     * @brief 设置是否验证对端证书
     * @param verify true=验证对端证书，false=不验证
     */
    void setPeerVerify(bool verify);

    // ---- 统计接口 ----

    /** @brief 获取SSL握手完成次数 */
    quint64 totalHandshakes() const;

    /** @brief 获取已发送字节总数 */
    quint64 totalBytesSent() const;

    /** @brief 获取已接收字节总数 */
    quint64 totalBytesReceived() const;

    /** @brief 获取错误计数 */
    quint64 errorCount() const;

    /** @brief 重置所有统计数据为零 */
    void resetStats();

private slots:
    /** @brief SSL加密通道建立完成回调 */
    void onEncrypted();

    /** @brief SSL错误回调 */
    void onSslErrors(const QList<QSslError>& errors);

    /** @brief 数据到达回调 */
    void onReadyRead();

    /** @brief socket连接状态变化回调 */
    void onStateChanged(QAbstractSocket::SocketState socketState);

private:
    /** @brief 更新连接状态 */
    void updateState(ConnectionState newState);

    // ---- 配置参数 ----
    QString m_host;                                 ///< 目标主机地址
    quint16 m_port = 0;                             ///< 目标端口号
    QString m_certPath;                             ///< 本地证书路径
    QString m_keyPath;                              ///< 私钥路径
    QString m_caPath;                               ///< CA证书路径
    bool m_peerVerify = false;                      ///< 是否验证对端证书
    ConnectionState m_state = ConnectionState::Disconnected; ///< 当前状态

    // ---- 网络资源 ----
    QSslSocket* m_socket = nullptr;                 ///< SSL加密socket

    // ---- 统计计数器 ----
    quint64 m_totalHandshakes = 0;                  ///< SSL握手完成次数
    quint64 m_totalBytesSent = 0;                   ///< 累计发送字节数
    quint64 m_totalBytesReceived = 0;               ///< 累计接收字节数
    quint64 m_errorCount = 0;                       ///< 错误发生次数
};

#endif // TLSCONNECTION_H
