#ifndef SERIAL_STATION_MODELS_H
#define SERIAL_STATION_MODELS_H

#include <QtCore/QByteArray>
#include <QtCore/QDateTime>
#include <QtCore/QString>
#include <QtCore/QVector>

namespace serial_station {

/**
 * @brief 日志方向，用于区分收发和内部状态。
 */
enum class SerialLogDirection {
    Rx,
    Tx,
    System,
    Error,
    Internal = System
};

/**
 * @brief 串口日志条目。
 */
struct SerialLogEntry {
    QDateTime timestamp = QDateTime::currentDateTime();
    SerialLogDirection direction = SerialLogDirection::Internal;
    QByteArray payload;
    QString note;
};

/**
 * @brief 串口会话状态。
 */
enum class SerialSessionState {
    Closed,
    Opening,
    Open,
    Error
};

/**
 * @brief Serial Station 档案中的快捷命令项。
 *
 * 只描述用户可复用的发送入口，不持有协议对象或串口资源。
 */
struct SerialProfileCommand {
    QString name;    ///< 用户可见命令名称
    QString payload; ///< 命令内容，按 mode 解释为 ASCII/HEX/协议命令
    QString mode;    ///< 发送模式: ascii/hex/protocol
};

} // namespace serial_station

#endif // SERIAL_STATION_MODELS_H
