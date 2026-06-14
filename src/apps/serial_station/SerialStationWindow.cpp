#include "apps/serial_station/SerialStationWindow.h"

#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QSignalBlocker>
#include <QtCore/QStandardPaths>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QVBoxLayout>

#include "apps/serial_station/SerialStationController.h"
#include "apps/serial_station/SerialStationModels.h"
#include "apps/serial_station/services/SerialProfileCatalogService.h"
#include "apps/serial_station/ui/SerialCommandPanel.h"
#include "apps/serial_station/ui/SerialLogPanel.h"
#include "apps/serial_station/ui/SerialMeasurementPanel.h"
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

QString normalizedMeasurementExportPath(const QString& filePath)
{
    const QFileInfo fileInfo(filePath);
    if (!fileInfo.suffix().isEmpty()) {
        return filePath;
    }

    return filePath + QStringLiteral(".csv");
}

} // namespace

SerialStationWindow::SerialStationWindow(QWidget* parent)
    : QWidget(parent)
    , m_controller(std::make_unique<SerialStationController>(this))
    , m_profileCatalog(std::make_unique<SerialProfileCatalogService>())
{
    setObjectName(QStringLiteral("serialStationWindow"));
    m_defaultProfileDirectory = m_profileCatalog->defaultProfileDirectory();

    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(10, 10, 10, 10);
    rootLayout->setSpacing(8);

    auto* profileToolbar = new QWidget(this);
    profileToolbar->setObjectName(QStringLiteral("serialProfileToolbar"));
    auto* profileToolbarLayout = new QHBoxLayout(profileToolbar);
    profileToolbarLayout->setContentsMargins(0, 0, 0, 0);
    profileToolbarLayout->setSpacing(8);

    auto* recentProfileLabel = new QLabel(tr("最近档案"), profileToolbar);
    recentProfileLabel->setObjectName(QStringLiteral("serialProfileRecentLabel"));
    m_recentProfileCombo = new QComboBox(profileToolbar);
    m_recentProfileCombo->setObjectName(QStringLiteral("serialProfileRecentCombo"));
    m_recentProfileCombo->setMinimumWidth(260);
    m_recentProfileCombo->setPlaceholderText(tr("暂无最近档案"));

    m_reloadLastProfileButton = new QPushButton(tr("重载上次"), profileToolbar);
    m_reloadLastProfileButton->setObjectName(QStringLiteral("serialProfileReloadLastButton"));
    m_pruneMissingProfilesButton = new QPushButton(tr("清理失效"), profileToolbar);
    m_pruneMissingProfilesButton->setObjectName(QStringLiteral("serialProfilePruneMissingButton"));
    m_clearRecentProfilesButton = new QPushButton(tr("清空最近"), profileToolbar);
    m_clearRecentProfilesButton->setObjectName(QStringLiteral("serialProfileClearRecentButton"));
    auto* importProfileDirectoryButton = new QPushButton(tr("导入目录"), profileToolbar);
    importProfileDirectoryButton->setObjectName(QStringLiteral("serialProfileImportDirectoryButton"));

    auto* saveProfileButton = new QPushButton(tr("保存档案"), profileToolbar);
    saveProfileButton->setObjectName(QStringLiteral("serialProfileSaveButton"));
    auto* loadProfileButton = new QPushButton(tr("加载档案"), profileToolbar);
    loadProfileButton->setObjectName(QStringLiteral("serialProfileLoadButton"));

    profileToolbarLayout->addWidget(recentProfileLabel);
    profileToolbarLayout->addWidget(m_recentProfileCombo, 1);
    profileToolbarLayout->addWidget(m_reloadLastProfileButton);
    profileToolbarLayout->addWidget(m_pruneMissingProfilesButton);
    profileToolbarLayout->addWidget(m_clearRecentProfilesButton);
    profileToolbarLayout->addWidget(importProfileDirectoryButton);
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
    m_measurementPanel = new SerialMeasurementPanel(leftPanel);

    leftLayout->addWidget(m_portPanel);
    leftLayout->addWidget(m_protocolPanel);
    leftLayout->addWidget(m_measurementPanel);
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
    connect(m_controller.get(), &SerialStationController::serialMeasurementUpdated,
            m_measurementPanel, &SerialMeasurementPanel::setSummaryLines);
    connect(m_controller.get(), &SerialStationController::serialMeasurementTrendUpdated,
            m_measurementPanel, &SerialMeasurementPanel::setTrendLines);
    connect(m_controller.get(), &SerialStationController::serialMeasurementFramesUpdated,
            m_measurementPanel, &SerialMeasurementPanel::setRecentFrames);
    connect(m_controller.get(), &SerialStationController::serialStateChanged,
            this, [this](SerialSessionState state) {
                m_commandPanel->setSendEnabled(state == SerialSessionState::Open);
                if (state == SerialSessionState::Open) {
                    m_logPanel->appendSystem(tr("串口已连接，命令发送已启用"));
                } else if (state == SerialSessionState::Error) {
                    m_logPanel->appendSystem(tr("串口连接异常，命令发送已停用"));
                } else if (state == SerialSessionState::Opening) {
                    m_logPanel->appendSystem(tr("串口连接中，命令发送暂不可用"));
                } else {
                    m_logPanel->appendSystem(tr("串口已断开，命令发送已停用"));
                }
            });
    connect(m_measurementPanel, &SerialMeasurementPanel::symbolCatalogImportRequested,
            this, &SerialStationWindow::selectMeasurementSymbolCatalog);
    connect(m_measurementPanel, &SerialMeasurementPanel::symbolCatalogBindRequested,
            this, &SerialStationWindow::bindMeasurementSymbolCatalog);
    connect(m_controller.get(), &SerialStationController::serialSymbolCatalogUpdated,
            this, &SerialStationWindow::applySymbolCatalogStatus);
    connect(m_measurementPanel, &SerialMeasurementPanel::exportRequested,
            this, [this]() {
                const QString defaultPath = QDir(defaultExportDirectory())
                    .filePath(m_controller->suggestedMeasurementExportFileName());
                const QString selectedPath = QFileDialog::getSaveFileName(
                    this,
                    tr("导出测量数据"),
                    defaultPath,
                    tr("CSV (*.csv)"));

                if (selectedPath.trimmed().isEmpty()) {
                    m_logPanel->appendSystem(tr("测量导出已取消"));
                    return;
                }

                SerialMeasurementExportRequest request;
                request.filePath = normalizedMeasurementExportPath(selectedPath);
                m_controller->exportMeasurementSnapshot(request);
            });
    connect(m_controller.get(), &SerialStationController::serialTxCounted,
            m_statusBar, &SerialStatusBar::incrementTx);
    connect(m_controller.get(), &SerialStationController::serialTxCounted,
            m_commandPanel, &SerialCommandPanel::confirmLastSentCommand);
    connect(m_controller.get(), &SerialStationController::serialRxCounted,
            m_statusBar, &SerialStatusBar::incrementRx);
    connect(m_controller.get(), &SerialStationController::serialErrorCounted,
            m_statusBar, &SerialStatusBar::incrementErrors);
    connect(m_controller.get(), &SerialStationController::serialCommandFailed,
            this, [this](const QString&, const QString&, const QString& message) {
                m_commandPanel->notifyCommandFailed(message);
            });
    connect(m_controller.get(), &SerialStationController::serialCommandFailedWithReason,
            this, [this](const QString&, const QString&, const QString& reason, const QString& message) {
                m_commandPanel->notifyCommandFailed(reason, message);
            });
    connect(m_portPanel, &SerialPortPanel::connectRequested,
            this, [this](const SerialPortConfig& config) {
                m_statusBar->setPortConfig(config);
                m_logPanel->appendSystem(tr("应用连接配置: %1").arg(config.endpointSummary()));
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
    connect(m_reloadLastProfileButton, &QPushButton::clicked,
            this, &SerialStationWindow::reloadLastProfile);
    connect(m_pruneMissingProfilesButton, &QPushButton::clicked,
            this, [this]() {
                pruneMissingProfiles();
            });
    connect(m_clearRecentProfilesButton, &QPushButton::clicked,
            this, &SerialStationWindow::clearRecentProfiles);
    connect(importProfileDirectoryButton, &QPushButton::clicked,
            this, &SerialStationWindow::importProfilesFromDefaultDirectory);
    connect(m_recentProfileCombo, QOverload<int>::of(&QComboBox::activated),
            this, &SerialStationWindow::loadSelectedRecentProfile);

    m_commandPanel->setSendEnabled(false);
    refreshProfileCatalogUi();
}

SerialStationWindow::~SerialStationWindow() = default;

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

bool SerialStationWindow::loadStartupLastProfile()
{
    pruneMissingProfiles(false);
    const QString path = lastProfilePath();
    if (path.isEmpty()) {
        m_logPanel->appendSystem(tr("没有可用于启动的上次配置档案"));
        refreshProfileCatalogUi();
        return false;
    }

    m_logPanel->appendSystem(tr("启动上次配置档案: %1").arg(path));
    return loadStartupProfile(path);
}

QStringList SerialStationWindow::recentProfilePaths() const
{
    return m_profileCatalog->recentProfilePaths();
}

QString SerialStationWindow::lastProfilePath() const
{
    return m_profileCatalog->lastProfilePath();
}

bool SerialStationWindow::reloadLastProfile()
{
    pruneMissingProfiles(false);
    const QString path = lastProfilePath();
    if (path.isEmpty()) {
        m_logPanel->appendSystem(tr("没有可重载的配置档案"));
        refreshProfileCatalogUi();
        return false;
    }

    const SerialProfileResult result = loadProfileFromFile(path);
    if (result.ok) {
        m_logPanel->appendSystem(tr("重载上次配置档案: %1").arg(path));
    }
    return result.ok;
}

int SerialStationWindow::pruneMissingProfiles()
{
    return pruneMissingProfiles(true);
}

bool SerialStationWindow::clearRecentProfiles()
{
    const bool hadRecentProfiles = !recentProfilePaths().isEmpty() || !lastProfilePath().isEmpty();
    const QString defaultDirectory = m_profileCatalog->defaultProfileDirectory();
    m_profileCatalog->clear();
    if (!defaultDirectory.isEmpty()) {
        m_profileCatalog->setDefaultProfileDirectory(defaultDirectory);
        m_defaultProfileDirectory = m_profileCatalog->defaultProfileDirectory();
    }
    refreshProfileCatalogUi();
    m_logPanel->appendSystem(tr("最近配置档案已清空"));
    return hadRecentProfiles;
}

int SerialStationWindow::importProfilesFromDefaultDirectory()
{
    const QString directoryPath = defaultProfileDirectory();
    const QStringList discoveredPaths = m_profileCatalog->discoverProfilePaths(directoryPath);
    const int importedCount = m_profileCatalog->importProfileDirectory(directoryPath);
    refreshProfileCatalogUi();
    if (importedCount > 0) {
        m_logPanel->appendSystem(tr("已导入配置档案: %1").arg(importedCount));
    } else if (!discoveredPaths.isEmpty()) {
        m_logPanel->appendSystem(tr("没有新的可导入配置档案"));
    } else {
        m_logPanel->appendSystem(tr("未发现可导入配置档案"));
    }
    return importedCount;
}

void SerialStationWindow::selectMeasurementSymbolCatalog()
{
    const QString defaultDirectory = m_measurementSymbolCatalogPath.isEmpty()
        ? (m_defaultProfileDirectory.isEmpty() ? defaultExportDirectory()
                                               : m_defaultProfileDirectory)
        : QFileInfo(m_measurementSymbolCatalogPath).absolutePath();
    const QString selectedPath = QFileDialog::getOpenFileName(
        this,
        tr("选择 AXF 符号目录"),
        defaultDirectory,
        tr("AXF/ELF 文件 (*.axf *.elf);;所有文件 (*)"));

    if (selectedPath.trimmed().isEmpty()) {
        m_logPanel->appendSystem(tr("AXF 符号目录导入已取消"));
        return;
    }

    const SerialSymbolCatalogResult result =
        m_controller->requestImportSymbolCatalogFromFile(selectedPath);
    if (!result.ok) {
        m_measurementSymbolCatalogPath.clear();
        m_measurementSymbolCatalogBound = false;
        m_measurementPanel->setSymbolCatalogStatus(tr("导入失败: %1")
                                                       .arg(result.errorMessage),
                                                   false);
        m_logPanel->appendSystem(
            tr("AXF 符号目录导入失败: %1").arg(result.errorMessage));
        return;
    }

    m_measurementSymbolCatalogBound = false;
    m_measurementSymbolCatalogPath = result.catalog.sourceFilePath;
    m_logPanel->appendSystem(
        tr("AXF 符号目录导入成功: %1 (%2 条)")
            .arg(QFileInfo(result.catalog.sourceFilePath).fileName())
            .arg(result.catalog.records.size()));
    applySymbolCatalogStatus();
}

void SerialStationWindow::bindMeasurementSymbolCatalog()
{
    const SerialSymbolCatalogSnapshot snapshot = m_controller->symbolCatalog();
    if (snapshot.records.isEmpty()) {
        m_logPanel->appendSystem(tr("请先导入 AXF 符号目录"));
        m_measurementSymbolCatalogBound = false;
        updateMeasurementSymbolCatalogStatus(snapshot.sourceFilePath, false);
        return;
    }

    m_measurementSymbolCatalogBound = true;
    m_measurementSymbolCatalogPath = snapshot.sourceFilePath;
    updateMeasurementSymbolCatalogStatus(snapshot.sourceFilePath, true);
    m_logPanel->appendSystem(
        tr("AXF 符号目录已绑定: %1")
            .arg(QFileInfo(snapshot.sourceFilePath).fileName()));
}

void SerialStationWindow::applySymbolCatalogStatus()
{
    applySymbolCatalogStatus(m_controller->symbolCatalogLines());
}

void SerialStationWindow::applySymbolCatalogStatus(const QStringList& lines)
{
    Q_UNUSED(lines)

    const SerialSymbolCatalogSnapshot snapshot = m_controller->symbolCatalog();
    updateMeasurementSymbolCatalogStatus(snapshot.sourceFilePath, m_measurementSymbolCatalogBound);
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

void SerialStationWindow::updateMeasurementSymbolCatalogStatus()
{
    updateMeasurementSymbolCatalogStatus(m_measurementSymbolCatalogPath, m_measurementSymbolCatalogBound);
}

void SerialStationWindow::updateMeasurementSymbolCatalogStatus(
    const QString& catalogPath,
    bool bound)
{
    const QString trimmedPath = catalogPath.trimmed();
    const SerialSymbolCatalogSnapshot snapshot = m_controller->symbolCatalog();
    if (trimmedPath.isEmpty() || snapshot.records.isEmpty()) {
        m_measurementSymbolCatalogPath.clear();
        m_measurementSymbolCatalogBound = false;
        m_measurementPanel->setSymbolCatalogStatus(tr("未导入 AXF 符号目录"), false);
        return;
    }

    const QString displayName = QFileInfo(trimmedPath).fileName();
    const QString label = bound ? tr("已绑定: %1") : tr("已导入: %1");
    m_measurementSymbolCatalogPath = trimmedPath;
    m_measurementSymbolCatalogBound = bound;
    m_measurementPanel->setSymbolCatalogStatus(
        label.arg(displayName.isEmpty() ? trimmedPath : displayName),
        !bound);
}

void SerialStationWindow::recordSuccessfulProfilePath(const QString& filePath)
{
    if (m_profileCatalog->recordProfilePath(filePath)) {
        refreshProfileCatalogUi();
    }
}

int SerialStationWindow::pruneMissingProfiles(bool logWhenEmpty)
{
    const int removedCount = m_profileCatalog->pruneMissingProfilePaths();
    refreshProfileCatalogUi();
    if (removedCount > 0) {
        m_logPanel->appendSystem(tr("已清理失效配置档案: %1").arg(removedCount));
    } else if (logWhenEmpty) {
        m_logPanel->appendSystem(tr("没有失效配置档案"));
    }
    return removedCount;
}

void SerialStationWindow::refreshProfileCatalogUi()
{
    if (!m_recentProfileCombo || !m_reloadLastProfileButton
        || !m_pruneMissingProfilesButton || !m_clearRecentProfilesButton) {
        return;
    }

    const QSignalBlocker blocker(m_recentProfileCombo);
    m_recentProfileCombo->clear();
    const QStringList paths = recentProfilePaths();
    for (const QString& path : paths) {
        const QString displayName = QFileInfo(path).fileName().isEmpty()
            ? path
            : QFileInfo(path).fileName();
        m_recentProfileCombo->addItem(displayName, path);
    }

    const bool hasLastProfile = !lastProfilePath().isEmpty();
    m_recentProfileCombo->setEnabled(!paths.isEmpty());
    m_reloadLastProfileButton->setEnabled(hasLastProfile);
    m_pruneMissingProfilesButton->setEnabled(!paths.isEmpty());
    m_clearRecentProfilesButton->setEnabled(!paths.isEmpty() || hasLastProfile);
}

void SerialStationWindow::loadSelectedRecentProfile(int index)
{
    const QString path = m_recentProfileCombo->itemData(index).toString();
    if (path.trimmed().isEmpty()) {
        return;
    }

    loadProfileFromFile(path);
}

} // namespace serial_station
