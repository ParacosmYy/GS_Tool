#include <QtTest/QtTest>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QToolButton>

#include "apps/serial_station/SerialStationController.h"
#include "apps/serial_station/SerialStationWindow.h"
#include "apps/serial_station/ui/SerialCommandPanel.h"
#include "apps/serial_station/ui/SerialLogPanel.h"
#include "apps/serial_station/ui/SerialPortPanel.h"
#include "apps/serial_station/ui/SerialStatusBar.h"

using serial_station::SerialCommandPanel;
using serial_station::SerialLogPanel;
using serial_station::SerialPortPanel;
using serial_station::SerialStationController;
using serial_station::SerialStationWindow;
using serial_station::SerialStatusBar;

class SerialStationWorkbenchTest : public QObject {
    Q_OBJECT

private slots:
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
};

void SerialStationWorkbenchTest::windowContainsWorkbenchRegions()
{
    SerialStationWindow window;

    QVERIFY(window.findChild<SerialPortPanel*>(QStringLiteral("serialPortPanel")) != nullptr);
    QVERIFY(window.findChild<SerialCommandPanel*>(QStringLiteral("serialCommandPanel")) != nullptr);
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

QTEST_MAIN(SerialStationWorkbenchTest)
#include "test_serial_station_workbench.moc"
