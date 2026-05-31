#ifndef ICONNECTION_H
#define ICONNECTION_H

#include <QObject>
#include <QByteArray>
#include "Constants.h"

// 连接抽象接口 - 所有连接类型(串口/TCP/UDP/RTT)都实现这个接口
// 这样上层功能(终端/协议解析/OTA)不需要关心底层连接方式
class IConnection : public QObject {
    Q_OBJECT

public:
    explicit IConnection(QObject* parent = nullptr)
        : QObject(parent) {}

    virtual ~IConnection() = default;

    // 获取连接类型
    virtual ConnectionType type() const = 0;

    // 获取连接名称 (如 "COM3" / "TCP:192.168.1.100:8080")
    virtual QString name() const = 0;

    // 获取当前连接状态
    virtual ConnectionState state() const = 0;

    // 打开连接，返回是否成功
    virtual bool open() = 0;

    // 关闭连接
    virtual void close() = 0;

    // 发送数据，返回实际发送的字节数，-1表示失败
    virtual qint64 write(const QByteArray& data) = 0;

signals:
    // 收到数据时发出
    void dataReceived(const QByteArray& data);

    // 连接状态变化时发出
    void stateChanged(ConnectionState newState);

    // 发生错误时发出
    void errorOccurred(const QString& errorMsg);
};

#endif // ICONNECTION_H
