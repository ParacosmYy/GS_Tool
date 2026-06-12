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

    /**
     * @brief 返回裁剪后的端口名，供连接和日志统一使用。
     * @return 标准化端口名
     */
    QString normalizedPortName() const;

    /**
     * @brief 返回端口名已裁剪的配置副本。
     * @return 标准化后的配置
     */
    SerialPortConfig normalized() const;

    /**
     * @brief 返回配置错误原因，配置合法时返回空字符串。
     * @return 本地化错误描述
     */
    QString validationError() const;

    /**
     * @brief 判断当前 UART 配置是否可用于打开串口。
     * @return 配置合法返回 true
     */
    bool isValid() const;

    /**
     * @brief 返回可读 UART 参数摘要。
     * @return 端口、波特率、数据位、校验、停止位、流控和控制线摘要
     */
    QString summary() const;
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
