#include "apps/serial_station/SerialStationApp.h"

#include "apps/serial_station/SerialStationWindow.h"

namespace serial_station {

QWidget* createSerialStationWidget(QWidget* parent)
{
    return new SerialStationWindow(parent);
}

} // namespace serial_station
