#ifndef SERIAL_STATION_CONTROLLER_H
#define SERIAL_STATION_CONTROLLER_H

#include <QtCore/QObject>

#include "apps/serial_station/core/SerialManager.h"
#include "apps/serial_station/protocols/SerialProtocolRegistry.h"

namespace serial_station {

/**
 * @brief Serial Station 控制器。
 *
 * 作为 UI 与 core/protocols 的唯一协调入口。
 */
class SerialStationController : public QObject {
    Q_OBJECT

public:
    explicit SerialStationController(QObject* parent = nullptr);

    SerialProtocolRegistry& protocols();
    SerialManager& serialManager();

private:
    SerialProtocolRegistry m_protocols;
    SerialManager m_serialManager;
};

} // namespace serial_station

#endif // SERIAL_STATION_CONTROLLER_H
