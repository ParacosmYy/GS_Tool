#ifndef TCPCONNECTION_H
#define TCPCONNECTION_H

#include "connection/IConnection.h"
#include <QTcpSocket>
#include <QTcpServer>

// TCP连接实现 - 支持Client和Server两种模式
// Client: 主动连接远程设备的TCP服务器
// Server: 本地监听端口，等待远程客户端连接
// 复用IConnection抽象接口，上层无需知道连接类型
class TcpConnection : public IConnection {
    Q_OBJECT

public:
    enum Mode {
        Client, // 客户端模式: 主动连接
        Server  // 服务器模式: 监听等待
    };

    explicit TcpConnection(QObject* parent = nullptr);
    ~TcpConnection() override;

    ConnectionType type() const override;
    QString name() const override;
    ConnectionState state() const override;
    bool open() override;
    void close() override;
    qint64 write(const QByteArray& data) override;
    void configure(const QVariantMap& params) override;

private slots:
    void onSocketConnected();
    void onSocketDisconnected();
    void onSocketReadyRead();
    void onSocketError(QAbstractSocket::SocketError error);
    void onNewConnection();

private:
    void updateState(ConnectionState newState);

    Mode m_mode = Client;
    QString m_host;
    quint16 m_port = 0;
    ConnectionState m_state = ConnectionState::Disconnected;

    QTcpSocket* m_socket = nullptr;
    QTcpServer* m_server = nullptr;
    QTcpSocket* m_clientSocket = nullptr; // Server模式下接受到的客户端连接
};

#endif // TCPCONNECTION_H
