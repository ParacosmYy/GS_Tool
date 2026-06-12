#ifndef SERIAL_STATION_CONSTANTS_H
#define SERIAL_STATION_CONSTANTS_H

#include <QtCore/QString>

namespace serial_station {

namespace serialStationConstants {

inline const QString kDefaultProtocolName = QStringLiteral("ascii_text");
inline const QString kAsciiFrameType = QStringLiteral("frame");
inline const QString kModbusFrameType = QStringLiteral("modbus_frame");
inline const QString kModbusErrorType = QStringLiteral("modbus_error");
inline const QString kLogType = QStringLiteral("log");
inline constexpr int kDefaultBaudRate = 115200;
inline constexpr int kDefaultReconnectIntervalMs = 1500;
inline constexpr int kMaxProtocolBufferBytes = 8192;

} // namespace serialStationConstants

} // namespace serial_station

#endif // SERIAL_STATION_CONSTANTS_H
