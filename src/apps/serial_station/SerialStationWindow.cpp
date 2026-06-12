#include "apps/serial_station/SerialStationWindow.h"

#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QStandardPaths>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QVBoxLayout>

#include "apps/serial_station/SerialStationController.h"
#include "apps/serial_station/ui/SerialCommandPanel.h"
#include "apps/serial_station/ui/SerialLogPanel.h"
#include "apps/serial_station/ui/SerialPortPanel.h"
#include "apps/serial_station/ui/SerialStatusBar.h"

namespace serial_station {

namespace {

QString defaultExportDirectory()
{
    const QString documents =
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    if (!documents.isEmpty()) {
        return documents;
    }

    return QDir::homePath();
}

SerialExportFormat exportFormatForPath(const QString& filePath)
{
    const QString suffix = QFileInfo(filePath).suffix().toLower();
    if (suffix == QStringLiteral("txt") || suffix == QStringLiteral("log")) {
        return SerialExportFormat::PlainText;
    }

    if (suffix == QStringLiteral("csv")) {
        return SerialExportFormat::Csv;
    }

    return SerialExportFormat::JsonLines;
}

QString normalizedExportPath(const QString& filePath)
{
    const QFileInfo fileInfo(filePath);
    if (!fileInfo.suffix().isEmpty()) {
        return filePath;
    }

    return filePath + QStringLiteral(".jsonl");
}

} // namespace

SerialStationWindow::SerialStationWindow(QWidget* parent)
    : QWidget(parent)
    , m_controller(std::make_unique<SerialStationController>(this))
{
    setObjectName(QStringLiteral("serialStationWindow"));

    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(10, 10, 10, 10);
    rootLayout->setSpacing(8);

    auto* workbench = new QSplitter(Qt::Horizontal, this);
    workbench->setObjectName(QStringLiteral("serialWorkbenchSplitter"));
    workbench->setChildrenCollapsible(false);

    m_portPanel = new SerialPortPanel(this);
    m_portPanel->setObjectName(QStringLiteral("serialPortPanel"));
    workbench->addWidget(m_portPanel);

    auto* centerPanel = new QWidget(this);
    centerPanel->setObjectName(QStringLiteral("serialWorkbenchCenter"));
    auto* centerLayout = new QVBoxLayout(centerPanel);
    centerLayout->setContentsMargins(0, 0, 0, 0);
    centerLayout->setSpacing(8);

    m_commandPanel = new SerialCommandPanel(centerPanel);
    m_logPanel = new SerialLogPanel(centerPanel);

    centerLayout->addWidget(m_commandPanel);
    centerLayout->addWidget(m_logPanel, 1);
    workbench->addWidget(centerPanel);
    workbench->setStretchFactor(0, 0);
    workbench->setStretchFactor(1, 1);
    workbench->setSizes({280, 720});

    m_statusBar = new SerialStatusBar(this);

    rootLayout->addWidget(workbench, 1);
    rootLayout->addWidget(m_statusBar);

    connect(m_portPanel, &SerialPortPanel::connectRequested,
            m_controller.get(), &SerialStationController::connectSerialPort);
    connect(m_portPanel, &SerialPortPanel::disconnectRequested,
            m_controller.get(), &SerialStationController::disconnectSerialPort);
    connect(m_controller.get(), &SerialStationController::serialStateChanged,
            m_portPanel, &SerialPortPanel::setSessionState);
    connect(m_controller.get(), &SerialStationController::serialErrorOccurred,
            m_portPanel, &SerialPortPanel::setErrorMessage);
    connect(m_controller.get(), &SerialStationController::serialStateChanged,
            m_statusBar, &SerialStatusBar::setSessionState);
    connect(m_controller.get(), &SerialStationController::serialErrorOccurred,
            this, [this](const QString& message) {
                m_statusBar->incrementErrors();
                m_logPanel->appendSystem(message);
            });
    connect(m_controller.get(), &SerialStationController::serialTxLogged,
            m_logPanel, &SerialLogPanel::appendTx);
    connect(m_controller.get(), &SerialStationController::serialRxLogged,
            m_logPanel, &SerialLogPanel::appendRx);
    connect(m_controller.get(), &SerialStationController::serialSystemLogged,
            m_logPanel, &SerialLogPanel::appendSystem);
    connect(m_controller.get(), &SerialStationController::serialTxCounted,
            m_statusBar, &SerialStatusBar::incrementTx);
    connect(m_controller.get(), &SerialStationController::serialTxCounted,
            m_commandPanel, &SerialCommandPanel::confirmLastSentCommand);
    connect(m_controller.get(), &SerialStationController::serialRxCounted,
            m_statusBar, &SerialStatusBar::incrementRx);
    connect(m_controller.get(), &SerialStationController::serialErrorCounted,
            m_statusBar, &SerialStatusBar::incrementErrors);
    connect(m_portPanel, &SerialPortPanel::connectRequested,
            this, [this](const SerialPortConfig& config) {
                m_statusBar->setPortConfig(config);
                m_logPanel->appendSystem(tr("应用串口配置: %1 @ %2")
                                             .arg(config.portName, QString::number(config.baudRate)));
            });
    connect(m_commandPanel, &SerialCommandPanel::sendRequested,
            m_controller.get(), &SerialStationController::sendCommand);
    connect(m_commandPanel, &SerialCommandPanel::quickCommandSelected,
            this, [this](const QString& command) {
                m_logPanel->appendSystem(tr("载入快捷命令: %1").arg(command));
            });
    connect(m_logPanel, &SerialLogPanel::cleared,
            m_statusBar, &SerialStatusBar::resetCounters);
    connect(m_logPanel, &SerialLogPanel::cleared,
            m_controller.get(), &SerialStationController::clearLogRecords);
    connect(m_logPanel, &SerialLogPanel::exportRequested,
            this, [this]() {
                const QString defaultPath = QDir(defaultExportDirectory())
                    .filePath(m_controller->suggestedExportFileName(SerialExportFormat::JsonLines));
                const QString selectedPath = QFileDialog::getSaveFileName(
                    this,
                    tr("导出串口日志"),
                    defaultPath,
                    tr("JSON Lines (*.jsonl);;Text (*.txt *.log);;CSV (*.csv)"));

                if (selectedPath.trimmed().isEmpty()) {
                    m_logPanel->appendSystem(tr("日志导出已取消"));
                    return;
                }

                SerialExportRequest request;
                request.filePath = normalizedExportPath(selectedPath);
                request.format = exportFormatForPath(request.filePath);
                m_controller->exportLogRecords(request);
            });
    connect(m_logPanel, &SerialLogPanel::replayRequested,
            this, [this]() {
                m_controller->previewReplayPlan();
            });
}

SerialStationWindow::~SerialStationWindow() = default;

} // namespace serial_station
