#include "app.h"
#include "window.h"

namespace serial_station {

QWidget* createSerialStationWidget(QWidget* parent) {
    return new SerialStationWindow(parent);
}

} // namespace serial_station
