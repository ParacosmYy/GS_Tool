#pragma once

#include <QWidget>

namespace serial_station {

class SerialStationWindow;

QWidget* createSerialStationWidget(QWidget* parent = nullptr);

} // namespace serial_station
