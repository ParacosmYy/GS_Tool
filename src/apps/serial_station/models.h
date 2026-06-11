#pragma once

#include <QByteArray>
#include <QDateTime>
#include <QString>

namespace serial_station {

enum class LogDirection {
    Rx,
    Tx,
    Internal
};

struct SerialStationLogEntry {
    QDateTime timestamp;
    LogDirection direction = LogDirection::Internal;
    QByteArray payload;
    QString note;

    SerialStationLogEntry() = default;
    SerialStationLogEntry(LogDirection dir, const QByteArray& data, QString message = {})
        : timestamp(QDateTime::currentDateTime()), direction(dir), payload(data), note(std::move(message)) {}
};

struct SerialStationFrameEvent {
    QByteArray protocolPayload;
    QString protocolName;
    QString command;
};

} // namespace serial_station
