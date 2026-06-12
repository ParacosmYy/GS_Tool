#ifndef SERIAL_SESSION_H
#define SERIAL_SESSION_H

#include <QtCore/QString>

#include "apps/serial_station/SerialStationConfig.h"
#include "apps/serial_station/SerialStationModels.h"

namespace serial_station {

/**
 * @brief 串口会话状态对象。
 *
 * 该类只保存状态，不直接操作 QSerialPort，也不更新 UI。
 */
class SerialSession {
public:
    void setConfig(const SerialPortConfig& config);
    SerialPortConfig config() const;

    void markOpening();
    void markOpen();
    void markClosed();
    void markError(const QString& message);

    SerialSessionState state() const;
    QString errorString() const;
    bool isOpen() const;

private:
    SerialPortConfig m_config;
    SerialSessionState m_state = SerialSessionState::Closed;
    QString m_errorString;
};

} // namespace serial_station

#endif // SERIAL_SESSION_H
