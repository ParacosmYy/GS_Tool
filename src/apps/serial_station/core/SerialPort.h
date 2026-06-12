#ifndef SERIAL_PORT_H
#define SERIAL_PORT_H

#include <QtCore/QByteArray>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtSerialPort/QSerialPort>

#include "apps/serial_station/SerialStationConfig.h"

namespace serial_station {

/**
 * @brief QSerialPort 的薄封装。
 *
 * 只负责 UART 参数应用、打开关闭和 bytes 收发，不解析协议。
 */
class SerialPort : public QObject {
    Q_OBJECT

public:
    explicit SerialPort(QObject* parent = nullptr);

    void configure(const SerialPortConfig& config);
    SerialPortConfig config() const;

    bool open();
    void close();
    bool isOpen() const;
    qint64 write(const QByteArray& bytes);
    QString errorString() const;

signals:
    void bytesReceived(const QByteArray& bytes);
    void errorOccurred(const QString& message);

private slots:
    void handleReadyRead();
    void handleError(QSerialPort::SerialPortError error);

private:
    QSerialPort m_port;
    SerialPortConfig m_config;
};

} // namespace serial_station

#endif // SERIAL_PORT_H
