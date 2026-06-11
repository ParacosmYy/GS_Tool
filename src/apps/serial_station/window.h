#pragma once

#include <QWidget>
#include <memory>

#include "controller.h"

namespace serial_station {

class SerialStationWindow : public QWidget {
    Q_OBJECT

public:
    explicit SerialStationWindow(QWidget* parent = nullptr);

private:
    std::unique_ptr<SerialStationController> m_controller;
};

} // namespace serial_station
