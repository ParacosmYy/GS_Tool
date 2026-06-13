#ifndef SERIAL_STATION_WINDOW_H
#define SERIAL_STATION_WINDOW_H

#include <QtWidgets/QWidget>

#include <memory>

namespace serial_station {

class SerialCommandPanel;
class SerialLogPanel;
class SerialProtocolPanel;
class SerialStationController;
class SerialStatusBar;
class SerialPortPanel;

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
    SerialPortPanel* m_portPanel = nullptr;
    SerialProtocolPanel* m_protocolPanel = nullptr;
    SerialCommandPanel* m_commandPanel = nullptr;
    SerialLogPanel* m_logPanel = nullptr;
    SerialStatusBar* m_statusBar = nullptr;
};

} // namespace serial_station

#endif // SERIAL_STATION_WINDOW_H
