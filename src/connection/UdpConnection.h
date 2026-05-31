#ifndef UDPCONNECTION_H
#define UDPCONNECTION_H

#include "connection/IConnection.h"
#include <QUdpSocket>
#include <QHostAddress>

// UDP连接实现 - 支持单播/广播收发数据报
// 复用IConnection抽象接口
class UdpConnection : public IConnection {
    Q_OBJECT

public:
    explicit UdpConnection(QObject* parent = nullptr);
    ~UdpConnection() override;

    ConnectionType type() const override;
    QString name() const override;
    ConnectionState state() const override;
    bool open() override;
    void close() override;
    qint64 write(const QByteArray& data) override;
    void configure(const QVariantMap& params) override;

private slots:
    void onReadyRead();
    void onError(QAbstractSocket::SocketError error);

private:
    void updateState(ConnectionState newState);

    quint16 m_localPort = 0;
    QHostAddress m_remoteHost;
    quint16 m_remotePort = 0;
    bool m_broadcast = false;
    ConnectionState m_state = ConnectionState::Disconnected;

    QUdpSocket* m_socket = nullptr;
};

#endif // UDPCONNECTION_H
