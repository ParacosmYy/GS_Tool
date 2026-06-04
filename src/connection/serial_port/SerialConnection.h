/**
 * @file SerialConnection.h
 * @brief 串口连接实现 — 适配器模式，封装QSerialPort到IConnection接口
 *
 * 职责: 串口参数配置(端口/波特率/数据位/校验/停止位/流控/DTR/RTS)、
 * 错误检测与分类(端口不存在/被占用/权限不足/意外断开)、信号翻译
 */
#ifndef SERIALCONNECTION_H
#define SERIALCONNECTION_H

#include <QSerialPort>
#include <QSerialPortInfo>
#include "connection/interface/IConnection.h"

/** @brief 串口连接实现 - 封装 QSerialPort。上层通过 IConnection 接口操作 */
class SerialConnection : public IConnection {
    Q_OBJECT

public:
    explicit SerialConnection(QObject* parent = nullptr);
    ~SerialConnection() override;

    // ---- IConnection 接口实现 ----
    ConnectionType type() const override { return ConnectionType::Serial; }
    QString name() const override;
    ConnectionState state() const override;
    bool open() override;
    void close() override;
    qint64 write(const QByteArray& data) override;
    void configure(const QVariantMap& params) override;

    // ---- 串口参数配置 ----
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
    void setDtr(bool enabled) override;
    void setRts(bool enabled) override;
    bool isDtr() const override;
    bool isRts() const override;
    void sendBreak(int duration = 100) override;
    PinoutSignals pinoutSignals() const override;
    static QList<QSerialPortInfo> availablePorts();

    // ---- 串口错误统计 ----
    SerialErrorCounters errorCounters() const override { return m_errorCounters; }
    void resetErrorCounters();

    // ---- 错误分类统计(累计) ----
    quint64 totalErrorsTracked() const { return m_totalErrorsTracked; }
    quint64 totalFramingErrors() const { return m_totalFramingErrors; }
    quint64 totalParityErrors() const { return m_totalParityErrors; }
    quint64 totalOverrunErrors() const { return m_totalOverrunErrors; }
    void resetErrorClassificationStats();

    // ---- 操作统计 ----
    quint64 totalOpens() const { return m_totalOpens; }
    quint64 totalCloses() const { return m_totalCloses; }
    quint64 totalBytesWritten() const { return m_totalBytesWritten; }
    quint64 totalBytesRead() const { return m_totalBytesRead; }
    quint64 totalWrites() const { return m_totalWrites; }
    quint64 errorCount() const { return m_errorCount; }
    quint64 totalConfigChanges() const { return m_totalConfigChanges; }
    quint64 totalPinChanges() const { return m_totalPinChanges; }
    void resetStats();

private slots:
    void onReadyRead();
    void onError(QSerialPort::SerialPortError error);
    void onBytesWritten(qint64 bytes);

private:
    QString translateError(QSerialPort::SerialPortError error);
    void queryPlatformErrors();

    QSerialPort m_serial;
    QString m_portName;
    ConnectionState m_state = ConnectionState::Disconnected;
    SerialErrorCounters m_errorCounters;
    quint64 m_totalErrorsTracked = 0;
    quint64 m_totalFramingErrors = 0;
    quint64 m_totalParityErrors = 0;
    quint64 m_totalOverrunErrors = 0;
    quint64 m_totalOpens = 0;
    quint64 m_totalCloses = 0;
    quint64 m_totalBytesWritten = 0;
    quint64 m_totalBytesRead = 0;
    quint64 m_totalWrites = 0;
    quint64 m_errorCount = 0;
    quint64 m_totalConfigChanges = 0;
    quint64 m_totalPinChanges = 0;
};

#endif // SERIALCONNECTION_H
