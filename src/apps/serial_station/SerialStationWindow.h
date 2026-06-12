#ifndef SERIAL_STATION_WINDOW_H
#define SERIAL_STATION_WINDOW_H

#include <QtWidgets/QWidget>

#include <memory>

namespace serial_station {

class SerialStationController;

/**
 * @brief Serial Station 顶层窗口骨架。
 *
 * 当前只负责持有 controller 和窗口对象名，不承载业务逻辑。
 */
class SerialStationWindow : public QWidget {
    Q_OBJECT

public:
    explicit SerialStationWindow(QWidget* parent = nullptr);
    ~SerialStationWindow() override;

private:
    std::unique_ptr<SerialStationController> m_controller;
};

} // namespace serial_station

#endif // SERIAL_STATION_WINDOW_H
