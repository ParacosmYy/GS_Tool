#include <QtTest/QtTest>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QTemporaryDir>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QToolButton>

#include "apps/serial_station/SerialStationController.h"
#include "apps/serial_station/SerialStationWindow.h"
#include "apps/serial_station/services/SerialProfileCatalogService.h"
#include "apps/serial_station/services/SerialProfileService.h"
#include "apps/serial_station/ui/SerialCommandPanel.h"
#include "apps/serial_station/ui/SerialLogPanel.h"
#include "apps/serial_station/ui/SerialPortPanel.h"
#include "apps/serial_station/ui/SerialProtocolPanel.h"
#include "apps/serial_station/ui/SerialStatusBar.h"

using serial_station::SerialCommandPanel;
using serial_station::SerialLogPanel;
using serial_station::SerialPortPanel;
using serial_station::SerialProfileCatalogService;
using serial_station::SerialProfileCommand;
using serial_station::SerialProfileService;
using serial_station::SerialProtocolPanel;
using serial_station::SerialStationProfile;
using serial_station::SerialStationController;
using serial_station::SerialStationWindow;
using serial_station::SerialStatusBar;

class SerialStationWorkbenchTest : public QObject {
    Q_OBJECT

private slots:
    void init();
    void cleanup();
    void windowContainsWorkbenchRegions();
    void commandModeComboOnlyShowsSupportedModes();
    void commandSendReportsControllerErrorWhenClosed();
    void protocolModeSendReportsControllerErrorWhenClosed();
    void commandSendIncrementsErrorCounterWhenClosed();
    void controllerReceiveUpdatesLogAndRxCounter();
    void controllerReceiveDoesNotIncrementErrorCounter();
    void controllerPartialReceiveShowsCacheStatus();
    void controllerSplitReceiveCompletesWorkbenchRxLine();
    void controllerMultipleReceiveLinesIncrementRxCounter();
    void logClearResetsRxCounter();
    void quickCommandFillsCommandInput();
    void logClearRemovesPreviewLines();
    void workbenchExposesCommandHistoryControls();
    void workbenchExposesProtocolSelectionControls();
    void protocolSelectionUpdatesControllerAndLog();
    void workbenchExposesProfileControls();
    void defaultProfileDirectoryStartsWithFallbackPath();
    void settingDefaultProfileDirectoryNormalizesPath();
    void settingBlankDefaultProfileDirectoryRestoresFallbackPath();
    void settingDefaultProfileDirectoryKeepsExistingProfileCatalog();
    void workbenchExposesRecentProfileControls();
    void workbenchExposesPruneMissingProfileControl();
    void loadingProfileAppliesWorkbenchState();
    void successfulProfileLoadUpdatesRecentCatalog();
    void failedProfileLoadDoesNotUpdateRecentCatalog();
    void reloadLastProfileAppliesPersistedCatalogPath();
    void reloadLastProfileFallsBackToNextExistingRecentProfile();
    void pruneMissingProfilesRemovesMissingCatalogEntries();
    void clickingPruneMissingProfilesLogsRemovedCount();
    void clickingPruneMissingProfilesLogsNoMissingWhenCatalogClean();
    void pruneMissingProfilesKeepsExistingProfileFiles();
    void clearRecentProfilesDisablesCatalogControls();
    void clearRecentProfilesPersistsAcrossWindows();
    void clickingClearRecentProfilesLogsSystemMessage();
    void clearRecentProfilesAllowsCatalogReuse();
    void selectingRecentProfileLoadsSelectedProfile();
    void clearRecentProfilesWhenEmptyReturnsFalse();
    void clearRecentProfilesDoesNotDeleteProfileFile();
    void clearRecentProfilesKeepsAppliedWorkbenchState();
    void savingProfileEnablesRecentCatalogControls();
    void reloadAfterClearReportsNoProfile();
    void loadingStartupProfileAppliesWorkbenchState();
    void loadingStartupLastProfileAppliesPersistedCatalogPath();
    void loadingStartupLastProfileWithoutCatalogReportsFailure();
    void loadingStartupLastProfileAfterClearReportsFailure();
    void loadingStartupLastProfileMissingFileKeepsWorkbenchState();
    void loadingStartupLastProfileFallsBackToNextExistingRecentProfile();
    void loadingStartupLastProfilePrunesAllMissingAndReportsFailure();
    void loadingStartupLastProfileInvalidFileKeepsPreviousState();
    void loadingStartupLastProfileRefreshesRecentControls();
    void loadingMissingStartupProfileReportsFailure();
    void loadingInvalidStartupProfileDoesNotPolluteWorkbenchState();
    void savingCurrentWorkbenchProfileWritesReusableFile();
    void loadingInvalidProfileDoesNotPolluteWorkbenchState();
};

void SerialStationWorkbenchTest::init()
{
    SerialProfileCatalogService catalog;
    catalog.clear();
}

void SerialStationWorkbenchTest::cleanup()
{
    SerialProfileCatalogService catalog;
    catalog.clear();
}

static SerialStationProfile sampleProfile()
{
    SerialStationProfile profile;
    profile.name = QStringLiteral("Factory Line A");
    profile.description = QStringLiteral("Regression profile");
    profile.tags = {QStringLiteral("factory"), QStringLiteral("line-a")};
    profile.port.portName = QStringLiteral("COM8");
    profile.port.baudRate = 57600;
    profile.port.dataBits = QSerialPort::Data7;
    profile.port.parity = QSerialPort::EvenParity;
    profile.port.stopBits = QSerialPort::TwoStop;
    profile.port.flowControl = QSerialPort::HardwareControl;
    profile.port.dtrEnabled = true;
    profile.port.rtsEnabled = true;
    profile.protocolName = QStringLiteral("custom_md");
    profile.sendMode = QStringLiteral("hex");
    profile.commands = {
        {QStringLiteral("Ping"), QStringLiteral("AA 55"), QStringLiteral("hex")},
        {QStringLiteral("Read Version"), QStringLiteral("AT+GMR"), QStringLiteral("ascii")},
    };
    return profile;
}

void SerialStationWorkbenchTest::windowContainsWorkbenchRegions()
{
    SerialStationWindow window;

    QVERIFY(window.findChild<SerialPortPanel*>(QStringLiteral("serialPortPanel")) != nullptr);
    QVERIFY(window.findChild<SerialCommandPanel*>(QStringLiteral("serialCommandPanel")) != nullptr);
    QVERIFY(window.findChild<SerialProtocolPanel*>(QStringLiteral("serialProtocolPanel")) != nullptr);
    QVERIFY(window.findChild<SerialLogPanel*>(QStringLiteral("serialLogPanel")) != nullptr);
    QVERIFY(window.findChild<SerialStatusBar*>(QStringLiteral("serialStatusBar")) != nullptr);
    QVERIFY(window.findChild<QPlainTextEdit*>(QStringLiteral("serialLogView")) != nullptr);
}

void SerialStationWorkbenchTest::commandModeComboOnlyShowsSupportedModes()
{
    SerialStationWindow window;
    auto* modeCombo = window.findChild<QComboBox*>(QStringLiteral("serialCommandModeCombo"));
    QVERIFY(modeCombo != nullptr);

    QStringList modeValues;
    for (int i = 0; i < modeCombo->count(); ++i) {
        modeValues.append(modeCombo->itemData(i).toString());
    }

    QVERIFY(modeValues.contains(QStringLiteral("ascii")));
    QVERIFY(modeValues.contains(QStringLiteral("hex")));
    QVERIFY(modeValues.contains(QStringLiteral("protocol")));
    QCOMPARE(modeValues.count(QStringLiteral("hex")), 1);
}

void SerialStationWorkbenchTest::commandSendReportsControllerErrorWhenClosed()
{
    SerialStationWindow window;
    auto* commandEdit = window.findChild<QLineEdit*>(QStringLiteral("serialCommandEdit"));
    auto* sendButton = window.findChild<QPushButton*>(QStringLiteral("serialCommandSendButton"));
    auto* logView = window.findChild<QPlainTextEdit*>(QStringLiteral("serialLogView"));
    QVERIFY(commandEdit != nullptr);
    QVERIFY(sendButton != nullptr);
    QVERIFY(logView != nullptr);

    commandEdit->setText(QStringLiteral("AT+GMR"));
    QTest::mouseClick(sendButton, Qt::LeftButton);

    QVERIFY(logView->toPlainText().contains(QStringLiteral("SYS")));
    QVERIFY(logView->toPlainText().contains(QStringLiteral("AT+GMR")));
    QVERIFY(logView->toPlainText().contains(QStringLiteral("串口未连接")));
}

void SerialStationWorkbenchTest::protocolModeSendReportsControllerErrorWhenClosed()
{
    SerialStationWindow window;
    auto* commandEdit = window.findChild<QLineEdit*>(QStringLiteral("serialCommandEdit"));
    auto* modeCombo = window.findChild<QComboBox*>(QStringLiteral("serialCommandModeCombo"));
    auto* sendButton = window.findChild<QPushButton*>(QStringLiteral("serialCommandSendButton"));
    auto* logView = window.findChild<QPlainTextEdit*>(QStringLiteral("serialLogView"));
    QVERIFY(commandEdit != nullptr);
    QVERIFY(modeCombo != nullptr);
    QVERIFY(sendButton != nullptr);
    QVERIFY(logView != nullptr);

    const int protocolIndex = modeCombo->findData(QStringLiteral("protocol"));
    QVERIFY(protocolIndex >= 0);
    modeCombo->setCurrentIndex(protocolIndex);

    commandEdit->setText(QStringLiteral("READ_ID"));
    QTest::mouseClick(sendButton, Qt::LeftButton);

    QVERIFY(logView->toPlainText().contains(QStringLiteral("READ_ID")));
    QVERIFY(logView->toPlainText().contains(QStringLiteral("串口未连接")));
}

void SerialStationWorkbenchTest::commandSendIncrementsErrorCounterWhenClosed()
{
    SerialStationWindow window;
    auto* commandEdit = window.findChild<QLineEdit*>(QStringLiteral("serialCommandEdit"));
    auto* sendButton = window.findChild<QPushButton*>(QStringLiteral("serialCommandSendButton"));
    auto* errorLabel = window.findChild<QLabel*>(QStringLiteral("serialStatusErrorLabel"));
    QVERIFY(commandEdit != nullptr);
    QVERIFY(sendButton != nullptr);
    QVERIFY(errorLabel != nullptr);

    QCOMPARE(errorLabel->text(), QStringLiteral("ERR: 0"));

    commandEdit->setText(QStringLiteral("PING"));
    QTest::mouseClick(sendButton, Qt::LeftButton);

    QCOMPARE(errorLabel->text(), QStringLiteral("ERR: 1"));
}

void SerialStationWorkbenchTest::controllerReceiveUpdatesLogAndRxCounter()
{
    SerialStationWindow window;
    auto* controller = window.findChild<SerialStationController*>();
    auto* logView = window.findChild<QPlainTextEdit*>(QStringLiteral("serialLogView"));
    auto* rxLabel = window.findChild<QLabel*>(QStringLiteral("serialStatusRxLabel"));
    QVERIFY(controller != nullptr);
    QVERIFY(logView != nullptr);
    QVERIFY(rxLabel != nullptr);

    QCOMPARE(rxLabel->text(), QStringLiteral("RX: 0"));

    controller->handleBytesReceived(QByteArray("OK\n"));

    QVERIFY(logView->toPlainText().contains(QStringLiteral("RX")));
    QVERIFY(logView->toPlainText().contains(QStringLiteral("OK")));
    QCOMPARE(rxLabel->text(), QStringLiteral("RX: 1"));
}

void SerialStationWorkbenchTest::controllerReceiveDoesNotIncrementErrorCounter()
{
    SerialStationWindow window;
    auto* controller = window.findChild<SerialStationController*>();
    auto* rxLabel = window.findChild<QLabel*>(QStringLiteral("serialStatusRxLabel"));
    auto* errorLabel = window.findChild<QLabel*>(QStringLiteral("serialStatusErrorLabel"));
    QVERIFY(controller != nullptr);
    QVERIFY(rxLabel != nullptr);
    QVERIFY(errorLabel != nullptr);

    controller->handleBytesReceived(QByteArray("READY\n"));

    QCOMPARE(rxLabel->text(), QStringLiteral("RX: 1"));
    QCOMPARE(errorLabel->text(), QStringLiteral("ERR: 0"));
}

void SerialStationWorkbenchTest::controllerPartialReceiveShowsCacheStatus()
{
    SerialStationWindow window;
    auto* controller = window.findChild<SerialStationController*>();
    auto* logView = window.findChild<QPlainTextEdit*>(QStringLiteral("serialLogView"));
    auto* rxLabel = window.findChild<QLabel*>(QStringLiteral("serialStatusRxLabel"));
    QVERIFY(controller != nullptr);
    QVERIFY(logView != nullptr);
    QVERIFY(rxLabel != nullptr);

    controller->handleBytesReceived(QByteArray("O"));

    QVERIFY(logView->toPlainText().contains(QStringLiteral("SYS")));
    QVERIFY(logView->toPlainText().contains(QStringLiteral("接收缓存")));
    QCOMPARE(rxLabel->text(), QStringLiteral("RX: 0"));
}

void SerialStationWorkbenchTest::controllerSplitReceiveCompletesWorkbenchRxLine()
{
    SerialStationWindow window;
    auto* controller = window.findChild<SerialStationController*>();
    auto* logView = window.findChild<QPlainTextEdit*>(QStringLiteral("serialLogView"));
    auto* rxLabel = window.findChild<QLabel*>(QStringLiteral("serialStatusRxLabel"));
    QVERIFY(controller != nullptr);
    QVERIFY(logView != nullptr);
    QVERIFY(rxLabel != nullptr);

    controller->handleBytesReceived(QByteArray("O"));
    controller->handleBytesReceived(QByteArray("K\n"));

    QVERIFY(logView->toPlainText().contains(QStringLiteral("接收缓存")));
    QVERIFY(logView->toPlainText().contains(QStringLiteral("OK")));
    QCOMPARE(rxLabel->text(), QStringLiteral("RX: 1"));
}

void SerialStationWorkbenchTest::controllerMultipleReceiveLinesIncrementRxCounter()
{
    SerialStationWindow window;
    auto* controller = window.findChild<SerialStationController*>();
    auto* logView = window.findChild<QPlainTextEdit*>(QStringLiteral("serialLogView"));
    auto* rxLabel = window.findChild<QLabel*>(QStringLiteral("serialStatusRxLabel"));
    QVERIFY(controller != nullptr);
    QVERIFY(logView != nullptr);
    QVERIFY(rxLabel != nullptr);

    controller->handleBytesReceived(QByteArray("OK\nERR\nREADY\n"));

    QVERIFY(logView->toPlainText().contains(QStringLiteral("OK")));
    QVERIFY(logView->toPlainText().contains(QStringLiteral("ERR")));
    QVERIFY(logView->toPlainText().contains(QStringLiteral("READY")));
    QCOMPARE(rxLabel->text(), QStringLiteral("RX: 3"));
}

void SerialStationWorkbenchTest::logClearResetsRxCounter()
{
    SerialStationWindow window;
    auto* controller = window.findChild<SerialStationController*>();
    auto* clearButton = window.findChild<QPushButton*>(QStringLiteral("serialLogClearButton"));
    auto* rxLabel = window.findChild<QLabel*>(QStringLiteral("serialStatusRxLabel"));
    QVERIFY(controller != nullptr);
    QVERIFY(clearButton != nullptr);
    QVERIFY(rxLabel != nullptr);

    controller->handleBytesReceived(QByteArray("OK\n"));
    QCOMPARE(rxLabel->text(), QStringLiteral("RX: 1"));

    QTest::mouseClick(clearButton, Qt::LeftButton);

    QCOMPARE(rxLabel->text(), QStringLiteral("RX: 0"));
}

void SerialStationWorkbenchTest::quickCommandFillsCommandInput()
{
    SerialStationWindow window;
    auto* commandEdit = window.findChild<QLineEdit*>(QStringLiteral("serialCommandEdit"));
    const QList<QToolButton*> quickButtons = window.findChildren<QToolButton*>(QStringLiteral("serialQuickCommandButton"));
    QVERIFY(commandEdit != nullptr);
    QVERIFY(!quickButtons.isEmpty());

    QTest::mouseClick(quickButtons.first(), Qt::LeftButton);

    QVERIFY(!commandEdit->text().trimmed().isEmpty());
}

void SerialStationWorkbenchTest::logClearRemovesPreviewLines()
{
    SerialStationWindow window;
    auto* commandEdit = window.findChild<QLineEdit*>(QStringLiteral("serialCommandEdit"));
    auto* sendButton = window.findChild<QPushButton*>(QStringLiteral("serialCommandSendButton"));
    auto* clearButton = window.findChild<QPushButton*>(QStringLiteral("serialLogClearButton"));
    auto* logView = window.findChild<QPlainTextEdit*>(QStringLiteral("serialLogView"));
    QVERIFY(commandEdit != nullptr);
    QVERIFY(sendButton != nullptr);
    QVERIFY(clearButton != nullptr);
    QVERIFY(logView != nullptr);

    commandEdit->setText(QStringLiteral("PING"));
    QTest::mouseClick(sendButton, Qt::LeftButton);
    QVERIFY(!logView->toPlainText().isEmpty());

    QTest::mouseClick(clearButton, Qt::LeftButton);
    QVERIFY(logView->toPlainText().isEmpty());
}

void SerialStationWorkbenchTest::workbenchExposesCommandHistoryControls()
{
    SerialStationWindow window;
    auto* historyCombo = window.findChild<QComboBox*>(QStringLiteral("serialCommandHistoryCombo"));
    auto* historyClearButton = window.findChild<QPushButton*>(QStringLiteral("serialHistoryClearButton"));
    QVERIFY(historyCombo != nullptr);
    QVERIFY(historyClearButton != nullptr);

    QVERIFY(!historyCombo->isEnabled());
    QVERIFY(!historyClearButton->isEnabled());
}

void SerialStationWorkbenchTest::workbenchExposesProtocolSelectionControls()
{
    SerialStationWindow window;
    auto* panel = window.findChild<SerialProtocolPanel*>(QStringLiteral("serialProtocolPanel"));
    auto* combo = window.findChild<QComboBox*>(QStringLiteral("serialProtocolCombo"));
    auto* status = window.findChild<QLabel*>(QStringLiteral("serialProtocolStatusLabel"));
    QVERIFY(panel != nullptr);
    QVERIFY(combo != nullptr);
    QVERIFY(status != nullptr);

    QVERIFY(combo->findData(QStringLiteral("ascii_text")) >= 0);
    QVERIFY(combo->findData(QStringLiteral("custom_md")) >= 0);
    QVERIFY(combo->findData(QStringLiteral("modbus_rtu")) >= 0);
    QCOMPARE(panel->activeProtocol(), QStringLiteral("ascii_text"));
    QVERIFY(status->text().contains(QStringLiteral("ascii_text")));
}

void SerialStationWorkbenchTest::protocolSelectionUpdatesControllerAndLog()
{
    SerialStationWindow window;
    auto* controller = window.findChild<SerialStationController*>();
    auto* combo = window.findChild<QComboBox*>(QStringLiteral("serialProtocolCombo"));
    auto* logView = window.findChild<QPlainTextEdit*>(QStringLiteral("serialLogView"));
    QVERIFY(controller != nullptr);
    QVERIFY(combo != nullptr);
    QVERIFY(logView != nullptr);

    const int customIndex = combo->findData(QStringLiteral("custom_md"));
    QVERIFY(customIndex >= 0);
    combo->setCurrentIndex(customIndex);
    emit combo->activated(customIndex);

    QCOMPARE(controller->activeProtocolName(), QStringLiteral("custom_md"));
    QVERIFY(logView->toPlainText().contains(QStringLiteral("已切换串口协议")));
    QVERIFY(logView->toPlainText().contains(QStringLiteral("custom_md")));
}

void SerialStationWorkbenchTest::workbenchExposesProfileControls()
{
    SerialStationWindow window;

    QVERIFY(window.findChild<QPushButton*>(QStringLiteral("serialProfileSaveButton")) != nullptr);
    QVERIFY(window.findChild<QPushButton*>(QStringLiteral("serialProfileLoadButton")) != nullptr);
}

void SerialStationWorkbenchTest::defaultProfileDirectoryStartsWithFallbackPath()
{
    SerialStationWindow window;

    QVERIFY(!window.defaultProfileDirectory().isEmpty());
    QVERIFY(QDir(window.defaultProfileDirectory()).isAbsolute());
}

void SerialStationWorkbenchTest::settingDefaultProfileDirectoryNormalizesPath()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QDir dir(tempDir.path());
    QVERIFY(dir.mkpath(QStringLiteral("profiles/line-a")));
    const QString rawPath = dir.filePath(QStringLiteral("profiles/./line-a"));
    const QString expectedPath = QDir::cleanPath(rawPath);

    SerialStationWindow window;
    window.setDefaultProfileDirectory(rawPath);

    QCOMPARE(window.defaultProfileDirectory(), expectedPath);
}

void SerialStationWorkbenchTest::settingBlankDefaultProfileDirectoryRestoresFallbackPath()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    SerialStationWindow window;
    const QString fallbackPath = window.defaultProfileDirectory();
    window.setDefaultProfileDirectory(QDir(tempDir.path()).filePath(QStringLiteral("profiles")));
    QVERIFY(window.defaultProfileDirectory() != fallbackPath);

    window.setDefaultProfileDirectory(QStringLiteral("   "));

    QCOMPARE(window.defaultProfileDirectory(), fallbackPath);
}

void SerialStationWorkbenchTest::settingDefaultProfileDirectoryKeepsExistingProfileCatalog()
{
    SerialProfileService service;
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString profilePath = QDir(tempDir.path()).filePath(QStringLiteral("line-a.edserialprofile"));
    QVERIFY(service.saveToFile(sampleProfile(), profilePath).ok);

    SerialStationWindow window;
    QVERIFY(window.loadProfileFromFile(profilePath).ok);
    window.setDefaultProfileDirectory(QDir(tempDir.path()).filePath(QStringLiteral("profiles")));

    QCOMPARE(window.recentProfilePaths(), QStringList({profilePath}));
    QCOMPARE(window.lastProfilePath(), profilePath);
}

void SerialStationWorkbenchTest::workbenchExposesRecentProfileControls()
{
    SerialStationWindow window;

    auto* recentCombo = window.findChild<QComboBox*>(QStringLiteral("serialProfileRecentCombo"));
    auto* reloadButton = window.findChild<QPushButton*>(QStringLiteral("serialProfileReloadLastButton"));
    auto* clearButton = window.findChild<QPushButton*>(QStringLiteral("serialProfileClearRecentButton"));
    auto* pruneButton = window.findChild<QPushButton*>(QStringLiteral("serialProfilePruneMissingButton"));
    QVERIFY(recentCombo != nullptr);
    QVERIFY(reloadButton != nullptr);
    QVERIFY(clearButton != nullptr);
    QVERIFY(pruneButton != nullptr);
    QVERIFY(!recentCombo->isEnabled());
    QVERIFY(!reloadButton->isEnabled());
    QVERIFY(!clearButton->isEnabled());
    QVERIFY(!pruneButton->isEnabled());
}

void SerialStationWorkbenchTest::workbenchExposesPruneMissingProfileControl()
{
    SerialStationWindow window;

    auto* pruneButton = window.findChild<QPushButton*>(QStringLiteral("serialProfilePruneMissingButton"));
    QVERIFY(pruneButton != nullptr);
    QCOMPARE(pruneButton->text(), QStringLiteral("清理失效"));
    QVERIFY(!pruneButton->isEnabled());
}

void SerialStationWorkbenchTest::loadingProfileAppliesWorkbenchState()
{
    SerialProfileService service;
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString path = QDir(tempDir.path()).filePath(QStringLiteral("line-a.edserialprofile"));
    QVERIFY(service.saveToFile(sampleProfile(), path).ok);

    SerialStationWindow window;
    const auto result = window.loadProfileFromFile(path);
    QVERIFY2(result.ok, qPrintable(result.errorMessage));

    auto* portPanel = window.findChild<SerialPortPanel*>(QStringLiteral("serialPortPanel"));
    auto* protocolPanel = window.findChild<SerialProtocolPanel*>(QStringLiteral("serialProtocolPanel"));
    auto* commandPanel = window.findChild<SerialCommandPanel*>(QStringLiteral("serialCommandPanel"));
    auto* logView = window.findChild<QPlainTextEdit*>(QStringLiteral("serialLogView"));
    QVERIFY(portPanel != nullptr);
    QVERIFY(protocolPanel != nullptr);
    QVERIFY(commandPanel != nullptr);
    QVERIFY(logView != nullptr);

    const auto config = portPanel->currentConfig();
    QCOMPARE(config.portName, QStringLiteral("COM8"));
    QCOMPARE(config.baudRate, 57600);
    QCOMPARE(config.dataBits, QSerialPort::Data7);
    QCOMPARE(config.parity, QSerialPort::EvenParity);
    QCOMPARE(config.stopBits, QSerialPort::TwoStop);
    QCOMPARE(config.flowControl, QSerialPort::HardwareControl);
    QVERIFY(config.dtrEnabled);
    QVERIFY(config.rtsEnabled);
    QCOMPARE(protocolPanel->activeProtocol(), QStringLiteral("custom_md"));
    QCOMPARE(commandPanel->sendMode(), QStringLiteral("hex"));
    QCOMPARE(commandPanel->commandText(), QStringLiteral("AA 55"));
    QCOMPARE(commandPanel->historyCount(), 2);
    QVERIFY(logView->toPlainText().contains(QStringLiteral("已加载配置档案")));
}

void SerialStationWorkbenchTest::successfulProfileLoadUpdatesRecentCatalog()
{
    SerialProfileService service;
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString path = QDir(tempDir.path()).filePath(QStringLiteral("line-a.edserialprofile"));
    QVERIFY(service.saveToFile(sampleProfile(), path).ok);

    SerialStationWindow window;
    QVERIFY(window.loadProfileFromFile(path).ok);

    QCOMPARE(window.lastProfilePath(), path);
    QCOMPARE(window.recentProfilePaths(), QStringList({path}));

    auto* recentCombo = window.findChild<QComboBox*>(QStringLiteral("serialProfileRecentCombo"));
    auto* reloadButton = window.findChild<QPushButton*>(QStringLiteral("serialProfileReloadLastButton"));
    QVERIFY(recentCombo != nullptr);
    QVERIFY(reloadButton != nullptr);
    QCOMPARE(recentCombo->count(), 1);
    QCOMPARE(recentCombo->itemData(0).toString(), path);
    QVERIFY(reloadButton->isEnabled());
}

void SerialStationWorkbenchTest::failedProfileLoadDoesNotUpdateRecentCatalog()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString missingPath = QDir(tempDir.path()).filePath(QStringLiteral("missing.edserialprofile"));

    SerialStationWindow window;
    const auto result = window.loadProfileFromFile(missingPath);

    QVERIFY(!result.ok);
    QVERIFY(window.recentProfilePaths().isEmpty());
    QCOMPARE(window.lastProfilePath(), QString());
}

void SerialStationWorkbenchTest::reloadLastProfileAppliesPersistedCatalogPath()
{
    SerialProfileService service;
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString path = QDir(tempDir.path()).filePath(QStringLiteral("line-a.edserialprofile"));
    QVERIFY(service.saveToFile(sampleProfile(), path).ok);

    {
        SerialStationWindow firstWindow;
        QVERIFY(firstWindow.loadProfileFromFile(path).ok);
    }

    SerialStationWindow secondWindow;
    QVERIFY(secondWindow.reloadLastProfile());

    auto* portPanel = secondWindow.findChild<SerialPortPanel*>(QStringLiteral("serialPortPanel"));
    auto* commandPanel = secondWindow.findChild<SerialCommandPanel*>(QStringLiteral("serialCommandPanel"));
    auto* logView = secondWindow.findChild<QPlainTextEdit*>(QStringLiteral("serialLogView"));
    QVERIFY(portPanel != nullptr);
    QVERIFY(commandPanel != nullptr);
    QVERIFY(logView != nullptr);
    QCOMPARE(secondWindow.lastProfilePath(), path);
    QCOMPARE(portPanel->currentConfig().portName, QStringLiteral("COM8"));
    QCOMPARE(commandPanel->commandText(), QStringLiteral("AA 55"));
    QVERIFY(logView->toPlainText().contains(QStringLiteral("重载上次配置档案")));
}

void SerialStationWorkbenchTest::reloadLastProfileFallsBackToNextExistingRecentProfile()
{
    SerialProfileService service;
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString existingPath = QDir(tempDir.path()).filePath(QStringLiteral("fallback.edserialprofile"));
    const QString missingPath = QDir(tempDir.path()).filePath(QStringLiteral("missing.edserialprofile"));
    SerialStationProfile fallbackProfile = sampleProfile();
    fallbackProfile.port.portName = QStringLiteral("COM15");
    fallbackProfile.commands = {{QStringLiteral("Fallback"), QStringLiteral("FALLBACK?"), QStringLiteral("ascii")}};
    fallbackProfile.sendMode = QStringLiteral("ascii");
    QVERIFY(service.saveToFile(fallbackProfile, existingPath).ok);

    SerialProfileCatalogService catalog;
    QVERIFY(catalog.recordProfilePath(existingPath));
    QVERIFY(catalog.recordProfilePath(missingPath));

    SerialStationWindow window;
    QVERIFY(window.reloadLastProfile());

    auto* portPanel = window.findChild<SerialPortPanel*>(QStringLiteral("serialPortPanel"));
    auto* commandPanel = window.findChild<SerialCommandPanel*>(QStringLiteral("serialCommandPanel"));
    auto* logView = window.findChild<QPlainTextEdit*>(QStringLiteral("serialLogView"));
    QVERIFY(portPanel != nullptr);
    QVERIFY(commandPanel != nullptr);
    QVERIFY(logView != nullptr);
    QCOMPARE(window.recentProfilePaths(), QStringList({existingPath}));
    QCOMPARE(window.lastProfilePath(), existingPath);
    QCOMPARE(portPanel->currentConfig().portName, QStringLiteral("COM15"));
    QCOMPARE(commandPanel->commandText(), QStringLiteral("FALLBACK?"));
    QVERIFY(logView->toPlainText().contains(QStringLiteral("已清理失效配置档案: 1")));
}

void SerialStationWorkbenchTest::pruneMissingProfilesRemovesMissingCatalogEntries()
{
    SerialProfileService service;
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString existingPath = QDir(tempDir.path()).filePath(QStringLiteral("line-a.edserialprofile"));
    const QString missingPath = QDir(tempDir.path()).filePath(QStringLiteral("missing.edserialprofile"));
    QVERIFY(service.saveToFile(sampleProfile(), existingPath).ok);

    SerialProfileCatalogService catalog;
    QVERIFY(catalog.recordProfilePath(existingPath));
    QVERIFY(catalog.recordProfilePath(missingPath));

    SerialStationWindow window;
    QCOMPARE(window.pruneMissingProfiles(), 1);

    auto* recentCombo = window.findChild<QComboBox*>(QStringLiteral("serialProfileRecentCombo"));
    auto* pruneButton = window.findChild<QPushButton*>(QStringLiteral("serialProfilePruneMissingButton"));
    QVERIFY(recentCombo != nullptr);
    QVERIFY(pruneButton != nullptr);
    QCOMPARE(window.recentProfilePaths(), QStringList({existingPath}));
    QCOMPARE(window.lastProfilePath(), existingPath);
    QCOMPARE(recentCombo->count(), 1);
    QCOMPARE(recentCombo->itemData(0).toString(), existingPath);
    QVERIFY(pruneButton->isEnabled());
}

void SerialStationWorkbenchTest::clickingPruneMissingProfilesLogsRemovedCount()
{
    SerialProfileService service;
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString existingPath = QDir(tempDir.path()).filePath(QStringLiteral("line-a.edserialprofile"));
    const QString missingPath = QDir(tempDir.path()).filePath(QStringLiteral("missing.edserialprofile"));
    QVERIFY(service.saveToFile(sampleProfile(), existingPath).ok);

    SerialProfileCatalogService catalog;
    QVERIFY(catalog.recordProfilePath(existingPath));
    QVERIFY(catalog.recordProfilePath(missingPath));

    SerialStationWindow window;
    auto* pruneButton = window.findChild<QPushButton*>(QStringLiteral("serialProfilePruneMissingButton"));
    auto* logView = window.findChild<QPlainTextEdit*>(QStringLiteral("serialLogView"));
    QVERIFY(pruneButton != nullptr);
    QVERIFY(logView != nullptr);
    QVERIFY(pruneButton->isEnabled());

    QTest::mouseClick(pruneButton, Qt::LeftButton);

    QCOMPARE(window.recentProfilePaths(), QStringList({existingPath}));
    QVERIFY(logView->toPlainText().contains(QStringLiteral("已清理失效配置档案: 1")));
}

void SerialStationWorkbenchTest::clickingPruneMissingProfilesLogsNoMissingWhenCatalogClean()
{
    SerialProfileService service;
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString existingPath = QDir(tempDir.path()).filePath(QStringLiteral("line-a.edserialprofile"));
    QVERIFY(service.saveToFile(sampleProfile(), existingPath).ok);

    SerialProfileCatalogService catalog;
    QVERIFY(catalog.recordProfilePath(existingPath));

    SerialStationWindow window;
    auto* pruneButton = window.findChild<QPushButton*>(QStringLiteral("serialProfilePruneMissingButton"));
    auto* logView = window.findChild<QPlainTextEdit*>(QStringLiteral("serialLogView"));
    QVERIFY(pruneButton != nullptr);
    QVERIFY(logView != nullptr);
    QVERIFY(pruneButton->isEnabled());

    QTest::mouseClick(pruneButton, Qt::LeftButton);

    QCOMPARE(window.recentProfilePaths(), QStringList({existingPath}));
    QVERIFY(logView->toPlainText().contains(QStringLiteral("没有失效配置档案")));
}

void SerialStationWorkbenchTest::pruneMissingProfilesKeepsExistingProfileFiles()
{
    SerialProfileService service;
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString existingPath = QDir(tempDir.path()).filePath(QStringLiteral("line-a.edserialprofile"));
    QVERIFY(service.saveToFile(sampleProfile(), existingPath).ok);

    SerialProfileCatalogService catalog;
    QVERIFY(catalog.recordProfilePath(existingPath));

    SerialStationWindow window;
    QCOMPARE(window.pruneMissingProfiles(), 0);

    QVERIFY(QFile::exists(existingPath));
    QCOMPARE(window.recentProfilePaths(), QStringList({existingPath}));
}

void SerialStationWorkbenchTest::clearRecentProfilesDisablesCatalogControls()
{
    SerialProfileService service;
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString path = QDir(tempDir.path()).filePath(QStringLiteral("line-a.edserialprofile"));
    QVERIFY(service.saveToFile(sampleProfile(), path).ok);

    SerialStationWindow window;
    QVERIFY(window.loadProfileFromFile(path).ok);

    auto* recentCombo = window.findChild<QComboBox*>(QStringLiteral("serialProfileRecentCombo"));
    auto* reloadButton = window.findChild<QPushButton*>(QStringLiteral("serialProfileReloadLastButton"));
    auto* clearButton = window.findChild<QPushButton*>(QStringLiteral("serialProfileClearRecentButton"));
    QVERIFY(recentCombo != nullptr);
    QVERIFY(reloadButton != nullptr);
    QVERIFY(clearButton != nullptr);

    QVERIFY(window.clearRecentProfiles());

    QVERIFY(window.recentProfilePaths().isEmpty());
    QCOMPARE(window.lastProfilePath(), QString());
    QCOMPARE(recentCombo->count(), 0);
    QVERIFY(!recentCombo->isEnabled());
    QVERIFY(!reloadButton->isEnabled());
    QVERIFY(!clearButton->isEnabled());
}

void SerialStationWorkbenchTest::clearRecentProfilesPersistsAcrossWindows()
{
    SerialProfileService service;
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString path = QDir(tempDir.path()).filePath(QStringLiteral("line-a.edserialprofile"));
    QVERIFY(service.saveToFile(sampleProfile(), path).ok);

    {
        SerialStationWindow window;
        QVERIFY(window.loadProfileFromFile(path).ok);
        QVERIFY(window.clearRecentProfiles());
    }

    SerialStationWindow reopenedWindow;

    QVERIFY(reopenedWindow.recentProfilePaths().isEmpty());
    QCOMPARE(reopenedWindow.lastProfilePath(), QString());
    auto* recentCombo = reopenedWindow.findChild<QComboBox*>(QStringLiteral("serialProfileRecentCombo"));
    auto* reloadButton = reopenedWindow.findChild<QPushButton*>(QStringLiteral("serialProfileReloadLastButton"));
    auto* clearButton = reopenedWindow.findChild<QPushButton*>(QStringLiteral("serialProfileClearRecentButton"));
    QVERIFY(recentCombo != nullptr);
    QVERIFY(reloadButton != nullptr);
    QVERIFY(clearButton != nullptr);
    QCOMPARE(recentCombo->count(), 0);
    QVERIFY(!reloadButton->isEnabled());
    QVERIFY(!clearButton->isEnabled());
}

void SerialStationWorkbenchTest::clickingClearRecentProfilesLogsSystemMessage()
{
    SerialProfileService service;
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString path = QDir(tempDir.path()).filePath(QStringLiteral("line-a.edserialprofile"));
    QVERIFY(service.saveToFile(sampleProfile(), path).ok);

    SerialStationWindow window;
    QVERIFY(window.loadProfileFromFile(path).ok);

    auto* clearButton = window.findChild<QPushButton*>(QStringLiteral("serialProfileClearRecentButton"));
    auto* logView = window.findChild<QPlainTextEdit*>(QStringLiteral("serialLogView"));
    QVERIFY(clearButton != nullptr);
    QVERIFY(logView != nullptr);
    QVERIFY(clearButton->isEnabled());

    QTest::mouseClick(clearButton, Qt::LeftButton);

    QVERIFY(window.recentProfilePaths().isEmpty());
    QVERIFY(logView->toPlainText().contains(QStringLiteral("最近配置档案已清空")));
}

void SerialStationWorkbenchTest::clearRecentProfilesAllowsCatalogReuse()
{
    SerialProfileService service;
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString firstPath = QDir(tempDir.path()).filePath(QStringLiteral("line-a.edserialprofile"));
    const QString secondPath = QDir(tempDir.path()).filePath(QStringLiteral("line-b.edserialprofile"));
    QVERIFY(service.saveToFile(sampleProfile(), firstPath).ok);
    SerialStationProfile secondProfile = sampleProfile();
    secondProfile.name = QStringLiteral("Factory Line B");
    secondProfile.port.portName = QStringLiteral("COM12");
    QVERIFY(service.saveToFile(secondProfile, secondPath).ok);

    SerialStationWindow window;
    QVERIFY(window.loadProfileFromFile(firstPath).ok);
    QVERIFY(window.clearRecentProfiles());
    QVERIFY(window.loadProfileFromFile(secondPath).ok);

    QCOMPARE(window.recentProfilePaths(), QStringList({secondPath}));
    QCOMPARE(window.lastProfilePath(), secondPath);
    auto* recentCombo = window.findChild<QComboBox*>(QStringLiteral("serialProfileRecentCombo"));
    QVERIFY(recentCombo != nullptr);
    QCOMPARE(recentCombo->count(), 1);
    QCOMPARE(recentCombo->itemData(0).toString(), secondPath);
}

void SerialStationWorkbenchTest::selectingRecentProfileLoadsSelectedProfile()
{
    SerialProfileService service;
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString firstPath = QDir(tempDir.path()).filePath(QStringLiteral("line-a.edserialprofile"));
    const QString secondPath = QDir(tempDir.path()).filePath(QStringLiteral("line-b.edserialprofile"));
    SerialStationProfile firstProfile = sampleProfile();
    firstProfile.port.portName = QStringLiteral("COM8");
    SerialStationProfile secondProfile = sampleProfile();
    secondProfile.name = QStringLiteral("Factory Line B");
    secondProfile.port.portName = QStringLiteral("COM12");
    secondProfile.commands = {{QStringLiteral("Status"), QStringLiteral("STATUS?"), QStringLiteral("ascii")}};
    secondProfile.sendMode = QStringLiteral("ascii");
    QVERIFY(service.saveToFile(firstProfile, firstPath).ok);
    QVERIFY(service.saveToFile(secondProfile, secondPath).ok);

    SerialStationWindow window;
    QVERIFY(window.loadProfileFromFile(firstPath).ok);
    QVERIFY(window.loadProfileFromFile(secondPath).ok);

    auto* recentCombo = window.findChild<QComboBox*>(QStringLiteral("serialProfileRecentCombo"));
    auto* portPanel = window.findChild<SerialPortPanel*>(QStringLiteral("serialPortPanel"));
    auto* commandPanel = window.findChild<SerialCommandPanel*>(QStringLiteral("serialCommandPanel"));
    QVERIFY(recentCombo != nullptr);
    QVERIFY(portPanel != nullptr);
    QVERIFY(commandPanel != nullptr);
    const int firstIndex = recentCombo->findData(firstPath);
    QVERIFY(firstIndex >= 0);

    recentCombo->setCurrentIndex(firstIndex);
    emit recentCombo->activated(firstIndex);

    QCOMPARE(portPanel->currentConfig().portName, QStringLiteral("COM8"));
    QCOMPARE(commandPanel->commandText(), QStringLiteral("AA 55"));
    QCOMPARE(window.lastProfilePath(), firstPath);
}

void SerialStationWorkbenchTest::clearRecentProfilesWhenEmptyReturnsFalse()
{
    SerialStationWindow window;

    auto* recentCombo = window.findChild<QComboBox*>(QStringLiteral("serialProfileRecentCombo"));
    auto* reloadButton = window.findChild<QPushButton*>(QStringLiteral("serialProfileReloadLastButton"));
    auto* clearButton = window.findChild<QPushButton*>(QStringLiteral("serialProfileClearRecentButton"));
    QVERIFY(recentCombo != nullptr);
    QVERIFY(reloadButton != nullptr);
    QVERIFY(clearButton != nullptr);

    QVERIFY(!window.clearRecentProfiles());

    QVERIFY(window.recentProfilePaths().isEmpty());
    QVERIFY(!recentCombo->isEnabled());
    QVERIFY(!reloadButton->isEnabled());
    QVERIFY(!clearButton->isEnabled());
}

void SerialStationWorkbenchTest::clearRecentProfilesDoesNotDeleteProfileFile()
{
    SerialProfileService service;
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString path = QDir(tempDir.path()).filePath(QStringLiteral("line-a.edserialprofile"));
    QVERIFY(service.saveToFile(sampleProfile(), path).ok);

    SerialStationWindow window;
    QVERIFY(window.loadProfileFromFile(path).ok);
    QVERIFY(QFile::exists(path));

    QVERIFY(window.clearRecentProfiles());

    QVERIFY(QFile::exists(path));
    const auto loadResult = service.loadFromFile(path);
    QVERIFY2(loadResult.ok, qPrintable(loadResult.errorMessage));
    QCOMPARE(loadResult.profile.name, QStringLiteral("Factory Line A"));
}

void SerialStationWorkbenchTest::clearRecentProfilesKeepsAppliedWorkbenchState()
{
    SerialProfileService service;
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString path = QDir(tempDir.path()).filePath(QStringLiteral("line-a.edserialprofile"));
    QVERIFY(service.saveToFile(sampleProfile(), path).ok);

    SerialStationWindow window;
    QVERIFY(window.loadProfileFromFile(path).ok);

    auto* portPanel = window.findChild<SerialPortPanel*>(QStringLiteral("serialPortPanel"));
    auto* protocolPanel = window.findChild<SerialProtocolPanel*>(QStringLiteral("serialProtocolPanel"));
    auto* commandPanel = window.findChild<SerialCommandPanel*>(QStringLiteral("serialCommandPanel"));
    QVERIFY(portPanel != nullptr);
    QVERIFY(protocolPanel != nullptr);
    QVERIFY(commandPanel != nullptr);

    QVERIFY(window.clearRecentProfiles());

    QCOMPARE(portPanel->currentConfig().portName, QStringLiteral("COM8"));
    QCOMPARE(protocolPanel->activeProtocol(), QStringLiteral("custom_md"));
    QCOMPARE(commandPanel->commandText(), QStringLiteral("AA 55"));
}

void SerialStationWorkbenchTest::savingProfileEnablesRecentCatalogControls()
{
    SerialStationWindow window;
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString path = QDir(tempDir.path()).filePath(QStringLiteral("saved.edserialprofile"));

    const auto writeResult = window.saveCurrentProfileToFile(path, QStringLiteral("Saved Default Profile"));
    QVERIFY2(writeResult.ok, qPrintable(writeResult.errorMessage));

    auto* recentCombo = window.findChild<QComboBox*>(QStringLiteral("serialProfileRecentCombo"));
    auto* reloadButton = window.findChild<QPushButton*>(QStringLiteral("serialProfileReloadLastButton"));
    auto* clearButton = window.findChild<QPushButton*>(QStringLiteral("serialProfileClearRecentButton"));
    QVERIFY(recentCombo != nullptr);
    QVERIFY(reloadButton != nullptr);
    QVERIFY(clearButton != nullptr);
    QCOMPARE(window.recentProfilePaths(), QStringList({path}));
    QVERIFY(recentCombo->isEnabled());
    QVERIFY(reloadButton->isEnabled());
    QVERIFY(clearButton->isEnabled());
}

void SerialStationWorkbenchTest::reloadAfterClearReportsNoProfile()
{
    SerialProfileService service;
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString path = QDir(tempDir.path()).filePath(QStringLiteral("line-a.edserialprofile"));
    QVERIFY(service.saveToFile(sampleProfile(), path).ok);

    SerialStationWindow window;
    QVERIFY(window.loadProfileFromFile(path).ok);
    QVERIFY(window.clearRecentProfiles());

    auto* logView = window.findChild<QPlainTextEdit*>(QStringLiteral("serialLogView"));
    QVERIFY(logView != nullptr);

    QVERIFY(!window.reloadLastProfile());

    QVERIFY(logView->toPlainText().contains(QStringLiteral("没有可重载的配置档案")));
}

void SerialStationWorkbenchTest::loadingStartupProfileAppliesWorkbenchState()
{
    SerialProfileService service;
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString path = QDir(tempDir.path()).filePath(QStringLiteral("startup.edserialprofile"));
    QVERIFY(service.saveToFile(sampleProfile(), path).ok);

    SerialStationWindow window;
    QVERIFY2(window.loadStartupProfile(path), "启动档案加载入口应复用工作台加载流程");

    auto* portPanel = window.findChild<SerialPortPanel*>(QStringLiteral("serialPortPanel"));
    auto* commandPanel = window.findChild<SerialCommandPanel*>(QStringLiteral("serialCommandPanel"));
    auto* logView = window.findChild<QPlainTextEdit*>(QStringLiteral("serialLogView"));
    QVERIFY(portPanel != nullptr);
    QVERIFY(commandPanel != nullptr);
    QVERIFY(logView != nullptr);
    QCOMPARE(portPanel->currentConfig().portName, QStringLiteral("COM8"));
    QCOMPARE(commandPanel->sendMode(), QStringLiteral("hex"));
    QVERIFY(logView->toPlainText().contains(QStringLiteral("启动档案")));
}

void SerialStationWorkbenchTest::loadingStartupLastProfileAppliesPersistedCatalogPath()
{
    SerialProfileService service;
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString path = QDir(tempDir.path()).filePath(QStringLiteral("startup-last.edserialprofile"));
    QVERIFY(service.saveToFile(sampleProfile(), path).ok);

    {
        SerialStationWindow recorderWindow;
        QVERIFY(recorderWindow.loadProfileFromFile(path).ok);
    }

    SerialStationWindow startupWindow;
    QVERIFY(startupWindow.loadStartupLastProfile());

    auto* portPanel = startupWindow.findChild<SerialPortPanel*>(QStringLiteral("serialPortPanel"));
    auto* commandPanel = startupWindow.findChild<SerialCommandPanel*>(QStringLiteral("serialCommandPanel"));
    auto* logView = startupWindow.findChild<QPlainTextEdit*>(QStringLiteral("serialLogView"));
    QVERIFY(portPanel != nullptr);
    QVERIFY(commandPanel != nullptr);
    QVERIFY(logView != nullptr);
    QCOMPARE(startupWindow.lastProfilePath(), path);
    QCOMPARE(portPanel->currentConfig().portName, QStringLiteral("COM8"));
    QCOMPARE(commandPanel->commandText(), QStringLiteral("AA 55"));
    QVERIFY(logView->toPlainText().contains(QStringLiteral("启动上次配置档案")));
}

void SerialStationWorkbenchTest::loadingStartupLastProfileWithoutCatalogReportsFailure()
{
    SerialStationWindow window;
    auto* logView = window.findChild<QPlainTextEdit*>(QStringLiteral("serialLogView"));
    QVERIFY(logView != nullptr);

    QVERIFY(!window.loadStartupLastProfile());

    QVERIFY(logView->toPlainText().contains(QStringLiteral("没有可用于启动的上次配置档案")));
}

void SerialStationWorkbenchTest::loadingStartupLastProfileAfterClearReportsFailure()
{
    SerialProfileService service;
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString path = QDir(tempDir.path()).filePath(QStringLiteral("line-a.edserialprofile"));
    QVERIFY(service.saveToFile(sampleProfile(), path).ok);

    SerialStationWindow window;
    QVERIFY(window.loadProfileFromFile(path).ok);
    QVERIFY(window.clearRecentProfiles());

    QVERIFY(!window.loadStartupLastProfile());
    QCOMPARE(window.lastProfilePath(), QString());
    auto* logView = window.findChild<QPlainTextEdit*>(QStringLiteral("serialLogView"));
    QVERIFY(logView != nullptr);
    QVERIFY(logView->toPlainText().contains(QStringLiteral("没有可用于启动的上次配置档案")));
}

void SerialStationWorkbenchTest::loadingStartupLastProfileMissingFileKeepsWorkbenchState()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString missingPath = QDir(tempDir.path()).filePath(QStringLiteral("missing.edserialprofile"));
    SerialProfileCatalogService catalog;
    QVERIFY(catalog.recordProfilePath(missingPath));

    SerialStationWindow window;
    auto* portPanel = window.findChild<SerialPortPanel*>(QStringLiteral("serialPortPanel"));
    auto* commandPanel = window.findChild<SerialCommandPanel*>(QStringLiteral("serialCommandPanel"));
    auto* logView = window.findChild<QPlainTextEdit*>(QStringLiteral("serialLogView"));
    QVERIFY(portPanel != nullptr);
    QVERIFY(commandPanel != nullptr);
    QVERIFY(logView != nullptr);
    const auto originalConfig = portPanel->currentConfig();
    const QString originalCommand = commandPanel->commandText();

    QVERIFY(!window.loadStartupLastProfile());

    QCOMPARE(portPanel->currentConfig().portName, originalConfig.portName);
    QCOMPARE(commandPanel->commandText(), originalCommand);
    QVERIFY(logView->toPlainText().contains(QStringLiteral("已清理失效配置档案: 1")));
    QVERIFY(logView->toPlainText().contains(QStringLiteral("没有可用于启动的上次配置档案")));
}

void SerialStationWorkbenchTest::loadingStartupLastProfileFallsBackToNextExistingRecentProfile()
{
    SerialProfileService service;
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString existingPath = QDir(tempDir.path()).filePath(QStringLiteral("fallback.edserialprofile"));
    const QString missingPath = QDir(tempDir.path()).filePath(QStringLiteral("missing.edserialprofile"));
    SerialStationProfile fallbackProfile = sampleProfile();
    fallbackProfile.name = QStringLiteral("Fallback Line");
    fallbackProfile.port.portName = QStringLiteral("COM16");
    fallbackProfile.commands = {{QStringLiteral("Fallback"), QStringLiteral("PING?"), QStringLiteral("ascii")}};
    fallbackProfile.sendMode = QStringLiteral("ascii");
    QVERIFY(service.saveToFile(fallbackProfile, existingPath).ok);

    SerialProfileCatalogService catalog;
    QVERIFY(catalog.recordProfilePath(existingPath));
    QVERIFY(catalog.recordProfilePath(missingPath));

    SerialStationWindow window;
    QVERIFY(window.loadStartupLastProfile());

    auto* portPanel = window.findChild<SerialPortPanel*>(QStringLiteral("serialPortPanel"));
    auto* commandPanel = window.findChild<SerialCommandPanel*>(QStringLiteral("serialCommandPanel"));
    auto* logView = window.findChild<QPlainTextEdit*>(QStringLiteral("serialLogView"));
    QVERIFY(portPanel != nullptr);
    QVERIFY(commandPanel != nullptr);
    QVERIFY(logView != nullptr);
    QCOMPARE(window.recentProfilePaths(), QStringList({existingPath}));
    QCOMPARE(window.lastProfilePath(), existingPath);
    QCOMPARE(portPanel->currentConfig().portName, QStringLiteral("COM16"));
    QCOMPARE(commandPanel->commandText(), QStringLiteral("PING?"));
    QVERIFY(logView->toPlainText().contains(QStringLiteral("已清理失效配置档案: 1")));
    QVERIFY(logView->toPlainText().contains(QStringLiteral("启动上次配置档案")));
}

void SerialStationWorkbenchTest::loadingStartupLastProfilePrunesAllMissingAndReportsFailure()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString firstMissing = QDir(tempDir.path()).filePath(QStringLiteral("first-missing.edserialprofile"));
    const QString secondMissing = QDir(tempDir.path()).filePath(QStringLiteral("second-missing.edserialprofile"));
    SerialProfileCatalogService catalog;
    QVERIFY(catalog.recordProfilePath(firstMissing));
    QVERIFY(catalog.recordProfilePath(secondMissing));

    SerialStationWindow window;
    QVERIFY(!window.loadStartupLastProfile());

    auto* recentCombo = window.findChild<QComboBox*>(QStringLiteral("serialProfileRecentCombo"));
    auto* reloadButton = window.findChild<QPushButton*>(QStringLiteral("serialProfileReloadLastButton"));
    auto* logView = window.findChild<QPlainTextEdit*>(QStringLiteral("serialLogView"));
    QVERIFY(recentCombo != nullptr);
    QVERIFY(reloadButton != nullptr);
    QVERIFY(logView != nullptr);
    QVERIFY(window.recentProfilePaths().isEmpty());
    QCOMPARE(window.lastProfilePath(), QString());
    QCOMPARE(recentCombo->count(), 0);
    QVERIFY(!reloadButton->isEnabled());
    QVERIFY(logView->toPlainText().contains(QStringLiteral("已清理失效配置档案: 2")));
    QVERIFY(logView->toPlainText().contains(QStringLiteral("没有可用于启动的上次配置档案")));
}

void SerialStationWorkbenchTest::loadingStartupLastProfileInvalidFileKeepsPreviousState()
{
    SerialProfileService service;
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString goodPath = QDir(tempDir.path()).filePath(QStringLiteral("good.edserialprofile"));
    const QString badPath = QDir(tempDir.path()).filePath(QStringLiteral("bad.edserialprofile"));
    QVERIFY(service.saveToFile(sampleProfile(), goodPath).ok);
    QFile badFile(badPath);
    QVERIFY(badFile.open(QIODevice::WriteOnly));
    badFile.write("{\"name\":");
    badFile.close();

    SerialStationWindow window;
    QVERIFY(window.loadProfileFromFile(goodPath).ok);
    SerialProfileCatalogService catalog;
    QVERIFY(catalog.recordProfilePath(badPath));

    QVERIFY(!window.loadStartupLastProfile());

    auto* portPanel = window.findChild<SerialPortPanel*>(QStringLiteral("serialPortPanel"));
    auto* commandPanel = window.findChild<SerialCommandPanel*>(QStringLiteral("serialCommandPanel"));
    auto* logView = window.findChild<QPlainTextEdit*>(QStringLiteral("serialLogView"));
    QVERIFY(portPanel != nullptr);
    QVERIFY(commandPanel != nullptr);
    QVERIFY(logView != nullptr);
    QCOMPARE(portPanel->currentConfig().portName, QStringLiteral("COM8"));
    QCOMPARE(commandPanel->commandText(), QStringLiteral("AA 55"));
    QVERIFY(logView->toPlainText().contains(QStringLiteral("启动档案加载失败")));
}

void SerialStationWorkbenchTest::loadingStartupLastProfileRefreshesRecentControls()
{
    SerialProfileService service;
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString path = QDir(tempDir.path()).filePath(QStringLiteral("line-a.edserialprofile"));
    QVERIFY(service.saveToFile(sampleProfile(), path).ok);

    SerialProfileCatalogService catalog;
    QVERIFY(catalog.recordProfilePath(path));

    SerialStationWindow window;
    QVERIFY(window.loadStartupLastProfile());

    auto* recentCombo = window.findChild<QComboBox*>(QStringLiteral("serialProfileRecentCombo"));
    auto* reloadButton = window.findChild<QPushButton*>(QStringLiteral("serialProfileReloadLastButton"));
    auto* clearButton = window.findChild<QPushButton*>(QStringLiteral("serialProfileClearRecentButton"));
    QVERIFY(recentCombo != nullptr);
    QVERIFY(reloadButton != nullptr);
    QVERIFY(clearButton != nullptr);
    QCOMPARE(recentCombo->count(), 1);
    QCOMPARE(recentCombo->itemData(0).toString(), path);
    QVERIFY(recentCombo->isEnabled());
    QVERIFY(reloadButton->isEnabled());
    QVERIFY(clearButton->isEnabled());
}

void SerialStationWorkbenchTest::loadingMissingStartupProfileReportsFailure()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString missingPath = QDir(tempDir.path()).filePath(QStringLiteral("missing.edserialprofile"));

    SerialStationWindow window;
    auto* portPanel = window.findChild<SerialPortPanel*>(QStringLiteral("serialPortPanel"));
    auto* commandPanel = window.findChild<SerialCommandPanel*>(QStringLiteral("serialCommandPanel"));
    auto* logView = window.findChild<QPlainTextEdit*>(QStringLiteral("serialLogView"));
    QVERIFY(portPanel != nullptr);
    QVERIFY(commandPanel != nullptr);
    QVERIFY(logView != nullptr);
    const auto originalConfig = portPanel->currentConfig();
    const QString originalCommand = commandPanel->commandText();

    QVERIFY(!window.loadStartupProfile(missingPath));

    const auto currentConfig = portPanel->currentConfig();
    QCOMPARE(currentConfig.portName, originalConfig.portName);
    QCOMPARE(currentConfig.baudRate, originalConfig.baudRate);
    QCOMPARE(currentConfig.dataBits, originalConfig.dataBits);
    QCOMPARE(currentConfig.parity, originalConfig.parity);
    QCOMPARE(currentConfig.stopBits, originalConfig.stopBits);
    QCOMPARE(currentConfig.flowControl, originalConfig.flowControl);
    QCOMPARE(commandPanel->commandText(), originalCommand);
    QVERIFY(logView->toPlainText().contains(QStringLiteral("启动档案加载失败")));
}

void SerialStationWorkbenchTest::loadingInvalidStartupProfileDoesNotPolluteWorkbenchState()
{
    SerialProfileService service;
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString goodPath = QDir(tempDir.path()).filePath(QStringLiteral("good.edserialprofile"));
    const QString badPath = QDir(tempDir.path()).filePath(QStringLiteral("bad.edserialprofile"));
    QVERIFY(service.saveToFile(sampleProfile(), goodPath).ok);
    QFile badFile(badPath);
    QVERIFY(badFile.open(QIODevice::WriteOnly));
    badFile.write("{\"commands\":[");
    badFile.close();

    SerialStationWindow window;
    QVERIFY(window.loadStartupProfile(goodPath));
    QVERIFY(!window.loadStartupProfile(badPath));

    auto* portPanel = window.findChild<SerialPortPanel*>(QStringLiteral("serialPortPanel"));
    auto* commandPanel = window.findChild<SerialCommandPanel*>(QStringLiteral("serialCommandPanel"));
    auto* protocolPanel = window.findChild<SerialProtocolPanel*>(QStringLiteral("serialProtocolPanel"));
    auto* logView = window.findChild<QPlainTextEdit*>(QStringLiteral("serialLogView"));
    QVERIFY(portPanel != nullptr);
    QVERIFY(commandPanel != nullptr);
    QVERIFY(protocolPanel != nullptr);
    QVERIFY(logView != nullptr);
    QCOMPARE(portPanel->currentConfig().portName, QStringLiteral("COM8"));
    QCOMPARE(protocolPanel->activeProtocol(), QStringLiteral("custom_md"));
    QCOMPARE(commandPanel->commandText(), QStringLiteral("AA 55"));
    QVERIFY(logView->toPlainText().contains(QStringLiteral("启动档案加载失败")));
}

void SerialStationWorkbenchTest::savingCurrentWorkbenchProfileWritesReusableFile()
{
    SerialProfileService service;
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString sourcePath = QDir(tempDir.path()).filePath(QStringLiteral("source.edserialprofile"));
    const QString savedPath = QDir(tempDir.path()).filePath(QStringLiteral("saved.edserialprofile"));
    QVERIFY(service.saveToFile(sampleProfile(), sourcePath).ok);

    SerialStationWindow window;
    QVERIFY(window.loadProfileFromFile(sourcePath).ok);
    const auto writeResult = window.saveCurrentProfileToFile(
        savedPath,
        QStringLiteral("Saved Profile"),
        QStringLiteral("Saved from workbench"),
        {QStringLiteral("saved")});
    QVERIFY2(writeResult.ok, qPrintable(writeResult.errorMessage));

    const auto loadResult = service.loadFromFile(savedPath);
    QVERIFY2(loadResult.ok, qPrintable(loadResult.errorMessage));
    QCOMPARE(loadResult.profile.name, QStringLiteral("Saved Profile"));
    QCOMPARE(loadResult.profile.description, QStringLiteral("Saved from workbench"));
    QCOMPARE(loadResult.profile.tags, QStringList({QStringLiteral("saved")}));
    QCOMPARE(loadResult.profile.port.portName, QStringLiteral("COM8"));
    QCOMPARE(loadResult.profile.protocolName, QStringLiteral("custom_md"));
    QCOMPARE(loadResult.profile.sendMode, QStringLiteral("hex"));
    QCOMPARE(loadResult.profile.commands.size(), 2);
    QCOMPARE(loadResult.profile.commands.first().payload, QStringLiteral("AA 55"));
}

void SerialStationWorkbenchTest::loadingInvalidProfileDoesNotPolluteWorkbenchState()
{
    SerialProfileService service;
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString goodPath = QDir(tempDir.path()).filePath(QStringLiteral("good.edserialprofile"));
    const QString badPath = QDir(tempDir.path()).filePath(QStringLiteral("bad.edserialprofile"));
    QVERIFY(service.saveToFile(sampleProfile(), goodPath).ok);
    QFile badFile(badPath);
    QVERIFY(badFile.open(QIODevice::WriteOnly));
    badFile.write("{\"name\":");
    badFile.close();

    SerialStationWindow window;
    QVERIFY(window.loadProfileFromFile(goodPath).ok);
    const auto result = window.loadProfileFromFile(badPath);
    QVERIFY(!result.ok);

    auto* portPanel = window.findChild<SerialPortPanel*>(QStringLiteral("serialPortPanel"));
    auto* commandPanel = window.findChild<SerialCommandPanel*>(QStringLiteral("serialCommandPanel"));
    QVERIFY(portPanel != nullptr);
    QVERIFY(commandPanel != nullptr);
    QCOMPARE(portPanel->currentConfig().portName, QStringLiteral("COM8"));
    QCOMPARE(commandPanel->commandText(), QStringLiteral("AA 55"));
}

QTEST_MAIN(SerialStationWorkbenchTest)
#include "test_serial_station_workbench.moc"
