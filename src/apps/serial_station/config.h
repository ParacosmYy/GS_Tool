#pragma once

#include <QString>
#include <QSerialPort>

namespace serial_station {

struct SerialPortConfig {
    QString portName;
    int baudRate = 115200;
    QSerialPort::DataBits dataBits = QSerialPort::Data8;
    QSerialPort::Parity parity = QSerialPort::NoParity;
    QSerialPort::StopBits stopBits = QSerialPort::OneStop;
    QSerialPort::FlowControl flowControl = QSerialPort::NoFlowControl;
    int readTimeoutMs = 1000;

    bool isValid() const {
        return !portName.trimmed().isEmpty() && baudRate > 0;
    }
};

struct SerialStationConfig {
    SerialPortConfig port;
    bool autoReconnect = false;
    int reconnectIntervalMs = 1500;
    int maxReconnectCount = 5;
};

} // namespace serial_station
