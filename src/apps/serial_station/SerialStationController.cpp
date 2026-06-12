#include "apps/serial_station/SerialStationController.h"

namespace serial_station {

SerialStationController::SerialStationController(QObject* parent)
    : QObject(parent)
    , m_serialManager(this)
{
    m_protocols.registerBuiltInProtocols();
}

SerialProtocolRegistry& SerialStationController::protocols()
{
    return m_protocols;
}

SerialManager& SerialStationController::serialManager()
{
    return m_serialManager;
}

} // namespace serial_station
