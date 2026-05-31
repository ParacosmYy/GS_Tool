#ifndef SERIALCONNECTION_H
#define SERIALCONNECTION_H

#include "IConnection.h"
#include <QSerialPort>
#include <QSerialPortInfo>

// 串口连接实现 - 封装QSerialPort
class SerialConnection : public IConnection {
    Q_OBJECT

public:
    explicit SerialConnection(QObject* parent = nullptr);
    ~SerialConnection() override;

    // IConnection接口实现
    ConnectionType type() const override { return ConnectionType::Serial; }
    QString name() const override;
    ConnectionState state() const override;
    bool open() override;
    void close() override;
    qint64 write(const QByteArray& data) override;

    // ---- 串口配置 ----

    void setPortName(const QString& portName);
    QString portName() const;

    void setBaudRate(qint32 baud);
    qint32 baudRate() const;

    void setDataBits(QSerialPort::DataBits bits);
    QSerialPort::DataBits dataBits() const;

    void setParity(QSerialPort::Parity parity);
    QSerialPort::Parity parity() const;

    void setStopBits(QSerialPort::StopBits bits);
    QSerialPort::StopBits stopBits() const;

    void setFlowControl(QSerialPort::FlowControl control);
    QSerialPort::FlowControl flowControl() const;

    void setDtr(bool enabled);
    void setRts(bool enabled);

    // 获取系统中可用的串口列表
    static QList<QSerialPortInfo> availablePorts();

private slots:
    // QSerialPort的readyRead信号处理
    void onReadyRead();
    void onError(QSerialPort::SerialPortError error);

private:
    QSerialPort m_serial;           // Qt串口对象
    QString m_portName;             // 端口名 如 "COM3"
    ConnectionState m_state = ConnectionState::Disconnected;
};

#endif // SERIALCONNECTION_H
