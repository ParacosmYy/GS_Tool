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
#include "apps/serial_station/services/SerialProfileCatalogService.h"
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
