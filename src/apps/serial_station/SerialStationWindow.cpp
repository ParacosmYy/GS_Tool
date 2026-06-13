#include "apps/serial_station/SerialStationWindow.h"

#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QStandardPaths>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QVBoxLayout>

#include "apps/serial_station/SerialStationController.h"
#include "apps/serial_station/ui/SerialCommandPanel.h"
#include "apps/serial_station/ui/SerialLogPanel.h"
#include "apps/serial_station/ui/SerialPortPanel.h"
#include "apps/serial_station/ui/SerialProtocolPanel.h"
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

QString normalizedProfilePath(const QString& filePath)
{
    const QFileInfo fileInfo(filePath);
    if (!fileInfo.suffix().isEmpty()) {
        return filePath;
    }

    return filePath + QStringLiteral(".edserialprofile");
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

    auto* profileToolbar = new QWidget(this);
    profileToolbar->setObjectName(QStringLiteral("serialProfileToolbar"));
    auto* profileToolbarLayout = new QHBoxLayout(profileToolbar);
    profileToolbarLayout->setContentsMargins(0, 0, 0, 0);
    profileToolbarLayout->setSpacing(8);

    auto* saveProfileButton = new QPushButton(tr("保存档案"), profileToolbar);
    saveProfileButton->setObjectName(QStringLiteral("serialProfileSaveButton"));
    auto* loadProfileButton = new QPushButton(tr("加载档案"), profileToolbar);
    loadProfileButton->setObjectName(QStringLiteral("serialProfileLoadButton"));

    profileToolbarLayout->addStretch();
    profileToolbarLayout->addWidget(loadProfileButton);
    profileToolbarLayout->addWidget(saveProfileButton);

    auto* workbench = new QSplitter(Qt::Horizontal, this);
    workbench->setObjectName(QStringLiteral("serialWorkbenchSplitter"));
    workbench->setChildrenCollapsible(false);

    auto* leftPanel = new QWidget(this);
    leftPanel->setObjectName(QStringLiteral("serialWorkbenchLeft"));
    auto* leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(8);

    m_portPanel = new SerialPortPanel(leftPanel);
    m_portPanel->setObjectName(QStringLiteral("serialPortPanel"));
    m_protocolPanel = new SerialProtocolPanel(leftPanel);
    m_protocolPanel->setProtocols(m_controller->availableProtocolNames(),
                                  m_controller->activeProtocolName());

    leftLayout->addWidget(m_portPanel);
    leftLayout->addWidget(m_protocolPanel);
    leftLayout->addStretch();
    workbench->addWidget(leftPanel);

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

    rootLayout->addWidget(profileToolbar);
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
    connect(m_protocolPanel, &SerialProtocolPanel::protocolSelected,
            m_controller.get(), &SerialStationController::setActiveProtocol);
    connect(m_controller.get(), &SerialStationController::activeProtocolChanged,
            m_protocolPanel, &SerialProtocolPanel::setActiveProtocol);
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
    connect(saveProfileButton, &QPushButton::clicked,
            this, &SerialStationWindow::saveProfileWithDialog);
    connect(loadProfileButton, &QPushButton::clicked,
            this, &SerialStationWindow::loadProfileWithDialog);
}

SerialStationWindow::~SerialStationWindow() = default;

SerialProfileWriteResult SerialStationWindow::saveCurrentProfileToFile(
    const QString& filePath,
    const QString& name,
    const QString& description,
    const QStringList& tags)
{
    const SerialStationProfile profile = collectCurrentProfile(name, description, tags);
    const SerialProfileWriteResult result = m_controller->saveProfileToFile(profile, filePath);
    if (result.ok) {
        m_logPanel->appendSystem(tr("已保存配置档案: %1").arg(result.filePath));
    } else {
        m_logPanel->appendSystem(tr("保存配置档案失败: %1").arg(result.errorMessage));
    }
    return result;
}

SerialProfileResult SerialStationWindow::loadProfileFromFile(const QString& filePath)
{
    const SerialProfileResult result = m_controller->loadProfileFromFile(filePath);
    if (!result.ok) {
        m_logPanel->appendSystem(tr("加载配置档案失败: %1").arg(result.errorMessage));
        return result;
    }

    applyProfileToUi(result.profile);
    m_logPanel->appendSystem(tr("已加载配置档案: %1").arg(result.profile.name));
    return result;
}

bool SerialStationWindow::loadStartupProfile(const QString& filePath)
{
    const SerialProfileResult result = loadProfileFromFile(filePath);
    if (result.ok) {
        m_logPanel->appendSystem(tr("启动档案已应用: %1").arg(result.profile.name));
    } else {
        m_logPanel->appendSystem(tr("启动档案加载失败: %1").arg(result.errorMessage));
    }
    return result.ok;
}

SerialStationProfile SerialStationWindow::collectCurrentProfile(
    const QString& name,
    const QString& description,
    const QStringList& tags) const
{
    SerialStationProfile profile;
    profile.name = name;
    profile.description = description;
    profile.tags = tags;
    profile.port = m_portPanel->currentConfig();
    profile.protocolName = m_protocolPanel->activeProtocol();
    profile.sendMode = m_commandPanel->sendMode();

    const QStringList history = m_commandPanel->historyCommands();
    for (int index = 0; index < history.size(); ++index) {
        SerialProfileCommand command;
        command.name = tr("命令 %1").arg(index + 1);
        command.payload = history.at(index);
        command.mode = profile.sendMode;
        profile.commands.append(command);
    }
    if (profile.commands.isEmpty() && !m_commandPanel->commandText().isEmpty()) {
        profile.commands.append({tr("当前命令"), m_commandPanel->commandText(), profile.sendMode});
    }
    return profile;
}

void SerialStationWindow::applyProfileToUi(const SerialStationProfile& profile)
{
    m_portPanel->applyConfig(profile.port);
    m_protocolPanel->setActiveProtocol(profile.protocolName);
    m_controller->setActiveProtocol(profile.protocolName);
    m_commandPanel->applyProfileCommands(profile.commands, profile.sendMode);
    m_statusBar->setPortConfig(profile.port);
}

void SerialStationWindow::saveProfileWithDialog()
{
    const QString defaultPath = QDir(defaultExportDirectory())
        .filePath(QStringLiteral("serial-station.edserialprofile"));
    const QString selectedPath = QFileDialog::getSaveFileName(
        this,
        tr("保存串口工站档案"),
        defaultPath,
        tr("Serial Station Profile (*.edserialprofile);;JSON (*.json)"));

    if (selectedPath.trimmed().isEmpty()) {
        m_logPanel->appendSystem(tr("配置档案保存已取消"));
        return;
    }

    const QString profileName = QFileInfo(selectedPath).completeBaseName();
    saveCurrentProfileToFile(normalizedProfilePath(selectedPath), profileName);
}

void SerialStationWindow::loadProfileWithDialog()
{
    const QString selectedPath = QFileDialog::getOpenFileName(
        this,
        tr("加载串口工站档案"),
        defaultExportDirectory(),
        tr("Serial Station Profile (*.edserialprofile);;JSON (*.json);;All Files (*.*)"));

    if (selectedPath.trimmed().isEmpty()) {
        m_logPanel->appendSystem(tr("配置档案加载已取消"));
        return;
    }

    loadProfileFromFile(selectedPath);
}

} // namespace serial_station
