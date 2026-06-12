#include "apps/serial_station/SerialStationConfig.h"

namespace serial_station {

bool SerialPortConfig::isValid() const
{
    return !portName.trimmed().isEmpty() && baudRate > 0;
}

bool SerialStationConfig::isReconnectEnabled() const
{
    return autoReconnect && reconnectIntervalMs > 0;
}

} // namespace serial_station
