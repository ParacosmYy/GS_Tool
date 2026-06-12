#include "apps/serial_station/SerialStationWindow.h"

#include <QtWidgets/QVBoxLayout>

#include "apps/serial_station/SerialStationController.h"
#include "apps/serial_station/ui/SerialPortPanel.h"

namespace serial_station {

SerialStationWindow::SerialStationWindow(QWidget* parent)
    : QWidget(parent)
    , m_controller(std::make_unique<SerialStationController>(this))
{
    setObjectName(QStringLiteral("serialStationWindow"));

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_portPanel = new SerialPortPanel(this);
    m_portPanel->setObjectName(QStringLiteral("serialPortPanel"));
    layout->addWidget(m_portPanel);

    connect(m_portPanel, &SerialPortPanel::connectRequested,
            m_controller.get(), &SerialStationController::connectSerialPort);
    connect(m_portPanel, &SerialPortPanel::disconnectRequested,
            m_controller.get(), &SerialStationController::disconnectSerialPort);
    connect(m_controller.get(), &SerialStationController::serialStateChanged,
            m_portPanel, &SerialPortPanel::setSessionState);
    connect(m_controller.get(), &SerialStationController::serialErrorOccurred,
            m_portPanel, &SerialPortPanel::setErrorMessage);
}

SerialStationWindow::~SerialStationWindow() = default;

} // namespace serial_station
