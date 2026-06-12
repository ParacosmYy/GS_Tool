#ifndef SERIAL_STATION_APP_H
#define SERIAL_STATION_APP_H

#include <QtWidgets/QWidget>

namespace serial_station {

/**
 * @brief 创建 Serial Station 顶层 widget。
 */
QWidget* createSerialStationWidget(QWidget* parent = nullptr);

} // namespace serial_station

#endif // SERIAL_STATION_APP_H
