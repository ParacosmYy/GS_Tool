#include "apps/serial_station/SerialStationWindow.h"

#include <QtWidgets/QVBoxLayout>

#include "apps/serial_station/SerialStationController.h"

namespace serial_station {

SerialStationWindow::SerialStationWindow(QWidget* parent)
    : QWidget(parent)
    , m_controller(std::make_unique<SerialStationController>(this))
{
    setObjectName(QStringLiteral("serialStationWindow"));

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
}

SerialStationWindow::~SerialStationWindow() = default;

} // namespace serial_station
