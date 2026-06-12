#include <QtTest/QtTest>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QToolButton>

#include "apps/serial_station/SerialStationWindow.h"
#include "apps/serial_station/ui/SerialCommandPanel.h"
#include "apps/serial_station/ui/SerialLogPanel.h"
#include "apps/serial_station/ui/SerialPortPanel.h"
#include "apps/serial_station/ui/SerialStatusBar.h"

using serial_station::SerialCommandPanel;
using serial_station::SerialLogPanel;
using serial_station::SerialPortPanel;
using serial_station::SerialStationWindow;
using serial_station::SerialStatusBar;

class SerialStationWorkbenchTest : public QObject {
    Q_OBJECT

private slots:
    void windowContainsWorkbenchRegions();
    void commandSendCreatesLogPreview();
    void quickCommandFillsCommandInput();
    void logClearRemovesPreviewLines();
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

void SerialStationWorkbenchTest::commandSendCreatesLogPreview()
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

    QVERIFY(logView->toPlainText().contains(QStringLiteral("TX")));
    QVERIFY(logView->toPlainText().contains(QStringLiteral("AT+GMR")));
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

QTEST_MAIN(SerialStationWorkbenchTest)
#include "test_serial_station_workbench.moc"
