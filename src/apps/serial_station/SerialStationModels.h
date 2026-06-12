#ifndef SERIAL_STATION_MODELS_H
#define SERIAL_STATION_MODELS_H

#include <QtCore/QByteArray>
#include <QtCore/QDateTime>
#include <QtCore/QString>

namespace serial_station {

/**
 * @brief 日志方向，用于区分收发和内部状态。
 */
enum class SerialLogDirection {
    Rx,
    Tx,
    Internal
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

} // namespace serial_station

#endif // SERIAL_STATION_MODELS_H
