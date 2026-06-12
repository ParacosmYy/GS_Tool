#ifndef SERIAL_STATION_CONFIG_H
#define SERIAL_STATION_CONFIG_H

#include <QtCore/QString>
#include <QtSerialPort/QSerialPort>

namespace serial_station {

/**
 * @brief Serial Station 的串口配置值对象。
 *
 * 该类型只描述 UART 参数，不直接打开串口，也不持有 QWidget。
 */
struct SerialPortConfig {
    QString portName;
    int baudRate = 115200;
    QSerialPort::DataBits dataBits = QSerialPort::Data8;
    QSerialPort::Parity parity = QSerialPort::NoParity;
    QSerialPort::StopBits stopBits = QSerialPort::OneStop;
    QSerialPort::FlowControl flowControl = QSerialPort::NoFlowControl;
    bool dtrEnabled = false;
    bool rtsEnabled = false;

    bool isValid() const;
};

/**
 * @brief Serial Station 的运行配置。
 */
struct SerialStationConfig {
    SerialPortConfig port;
    bool autoReconnect = false;
    int reconnectIntervalMs = 1500;

    bool isReconnectEnabled() const;
};

} // namespace serial_station

#endif // SERIAL_STATION_CONFIG_H
