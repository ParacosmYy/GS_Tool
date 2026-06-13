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
    void workbenchExposesRecentProfileControls();
    void loadingProfileAppliesWorkbenchState();
    void successfulProfileLoadUpdatesRecentCatalog();
    void failedProfileLoadDoesNotUpdateRecentCatalog();
    void reloadLastProfileAppliesPersistedCatalogPath();
    void loadingStartupProfileAppliesWorkbenchState();
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

void SerialStationWorkbenchTest::workbenchExposesRecentProfileControls()
{
    SerialStationWindow window;

    auto* recentCombo = window.findChild<QComboBox*>(QStringLiteral("serialProfileRecentCombo"));
    auto* reloadButton = window.findChild<QPushButton*>(QStringLiteral("serialProfileReloadLastButton"));
    QVERIFY(recentCombo != nullptr);
    QVERIFY(reloadButton != nullptr);
    QVERIFY(!reloadButton->isEnabled());
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
