#include <QtTest/QtTest>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QToolButton>

#include "apps/serial_station/ui/SerialCommandHistoryModel.h"
#include "apps/serial_station/ui/SerialCommandPanel.h"
#include "apps/serial_station/SerialStationModels.h"

using serial_station::SerialCommandHistoryModel;
using serial_station::SerialCommandPanel;
using serial_station::SerialProfileCommand;

class SerialCommandPanelTest : public QObject {
    Q_OBJECT

private slots:
    void modeComboExposesAsciiHexAndProtocol();
    void defaultModeIsAscii();
    void sendModeTracksSelectedItem_data();
    void sendModeTracksSelectedItem();
    void sendButtonEmitsTrimmedCommandAndMode();
    void hexModeSendEmitsHexMode();
    void emptyCommandDoesNotEmitSendRequest();
    void quickCommandFillsInputAndEmitsSelection();
    void disablingSendDisablesInputsAndQuickButtons();
    void reEnableSendKeepsButtonDisabledUntilCommandExists();
    void historyModelRejectsEmptyCommands();
    void historyModelMovesDuplicateToFrontAndCountsUse();
    void historyModelKeepsModeSpecificEntries();
    void historyModelTrimsOldestCommand();
    void recordSentCommandUpdatesHistoryCombo();
    void historyComboAppliesCommandAndMode();
    void clearHistoryRemovesItemsWithoutChangingInput();
    void sendIntentDoesNotRecordHistoryUntilConfirmed();
    void confirmedSendRecordsTrimmedCommand();
    void disablingSendDisablesHistorySelectionButKeepsClearAvailable();
    void applyProfileCommandsRestoresDefaultModeAndHistory();
    void applyEmptyProfileCommandsClearsHistoryAndInput();
};

void SerialCommandPanelTest::modeComboExposesAsciiHexAndProtocol()
{
    SerialCommandPanel panel;
    auto* modeCombo = panel.findChild<QComboBox*>(QStringLiteral("serialCommandModeCombo"));
    QVERIFY(modeCombo != nullptr);

    QStringList modes;
    for (int i = 0; i < modeCombo->count(); ++i) {
        modes.append(modeCombo->itemData(i).toString());
    }

    QCOMPARE(modes.count(), 3);
    QCOMPARE(modes.at(0), QStringLiteral("ascii"));
    QCOMPARE(modes.at(1), QStringLiteral("hex"));
    QCOMPARE(modes.at(2), QStringLiteral("protocol"));
}

void SerialCommandPanelTest::defaultModeIsAscii()
{
    SerialCommandPanel panel;

    QCOMPARE(panel.sendMode(), QStringLiteral("ascii"));
}

void SerialCommandPanelTest::sendModeTracksSelectedItem_data()
{
    QTest::addColumn<QString>("mode");

    QTest::newRow("ascii") << QStringLiteral("ascii");
    QTest::newRow("hex") << QStringLiteral("hex");
    QTest::newRow("protocol") << QStringLiteral("protocol");
}

void SerialCommandPanelTest::sendModeTracksSelectedItem()
{
    QFETCH(QString, mode);

    SerialCommandPanel panel;
    auto* modeCombo = panel.findChild<QComboBox*>(QStringLiteral("serialCommandModeCombo"));
    QVERIFY(modeCombo != nullptr);

    const int index = modeCombo->findData(mode);
    QVERIFY(index >= 0);
    modeCombo->setCurrentIndex(index);

    QCOMPARE(panel.sendMode(), mode);
}

void SerialCommandPanelTest::sendButtonEmitsTrimmedCommandAndMode()
{
    SerialCommandPanel panel;
    auto* commandEdit = panel.findChild<QLineEdit*>(QStringLiteral("serialCommandEdit"));
    auto* sendButton = panel.findChild<QPushButton*>(QStringLiteral("serialCommandSendButton"));
    QVERIFY(commandEdit != nullptr);
    QVERIFY(sendButton != nullptr);

    commandEdit->setText(QStringLiteral("  AT+GMR  "));

    QSignalSpy spy(&panel, &SerialCommandPanel::sendRequested);
    QVERIFY(spy.isValid());
    QTest::mouseClick(sendButton, Qt::LeftButton);

    QCOMPARE(spy.count(), 1);
    const QList<QVariant> args = spy.takeFirst();
    QCOMPARE(args.at(0).toString(), QStringLiteral("AT+GMR"));
    QCOMPARE(args.at(1).toString(), QStringLiteral("ascii"));
}

void SerialCommandPanelTest::hexModeSendEmitsHexMode()
{
    SerialCommandPanel panel;
    auto* commandEdit = panel.findChild<QLineEdit*>(QStringLiteral("serialCommandEdit"));
    auto* modeCombo = panel.findChild<QComboBox*>(QStringLiteral("serialCommandModeCombo"));
    auto* sendButton = panel.findChild<QPushButton*>(QStringLiteral("serialCommandSendButton"));
    QVERIFY(commandEdit != nullptr);
    QVERIFY(modeCombo != nullptr);
    QVERIFY(sendButton != nullptr);

    const int hexIndex = modeCombo->findData(QStringLiteral("hex"));
    QVERIFY(hexIndex >= 0);
    modeCombo->setCurrentIndex(hexIndex);
    commandEdit->setText(QStringLiteral("01 03 00 00 00 02"));

    QSignalSpy spy(&panel, &SerialCommandPanel::sendRequested);
    QVERIFY(spy.isValid());
    QTest::mouseClick(sendButton, Qt::LeftButton);

    QCOMPARE(spy.count(), 1);
    const QList<QVariant> args = spy.takeFirst();
    QCOMPARE(args.at(0).toString(), QStringLiteral("01 03 00 00 00 02"));
    QCOMPARE(args.at(1).toString(), QStringLiteral("hex"));
}

void SerialCommandPanelTest::emptyCommandDoesNotEmitSendRequest()
{
    SerialCommandPanel panel;
    auto* commandEdit = panel.findChild<QLineEdit*>(QStringLiteral("serialCommandEdit"));
    auto* sendButton = panel.findChild<QPushButton*>(QStringLiteral("serialCommandSendButton"));
    QVERIFY(commandEdit != nullptr);
    QVERIFY(sendButton != nullptr);

    commandEdit->setText(QStringLiteral("   "));

    QSignalSpy spy(&panel, &SerialCommandPanel::sendRequested);
    QVERIFY(spy.isValid());
    QTest::mouseClick(sendButton, Qt::LeftButton);

    QCOMPARE(spy.count(), 0);
}

void SerialCommandPanelTest::quickCommandFillsInputAndEmitsSelection()
{
    SerialCommandPanel panel;
    auto* commandEdit = panel.findChild<QLineEdit*>(QStringLiteral("serialCommandEdit"));
    const QList<QToolButton*> quickButtons =
        panel.findChildren<QToolButton*>(QStringLiteral("serialQuickCommandButton"));
    QVERIFY(commandEdit != nullptr);
    QVERIFY(!quickButtons.isEmpty());

    QSignalSpy spy(&panel, &SerialCommandPanel::quickCommandSelected);
    QVERIFY(spy.isValid());
    QTest::mouseClick(quickButtons.first(), Qt::LeftButton);

    QCOMPARE(spy.count(), 1);
    QCOMPARE(commandEdit->text(), spy.takeFirst().at(0).toString());
    QVERIFY(!commandEdit->text().trimmed().isEmpty());
}

void SerialCommandPanelTest::disablingSendDisablesInputsAndQuickButtons()
{
    SerialCommandPanel panel;
    auto* commandEdit = panel.findChild<QLineEdit*>(QStringLiteral("serialCommandEdit"));
    auto* modeCombo = panel.findChild<QComboBox*>(QStringLiteral("serialCommandModeCombo"));
    auto* sendButton = panel.findChild<QPushButton*>(QStringLiteral("serialCommandSendButton"));
    const QList<QToolButton*> quickButtons =
        panel.findChildren<QToolButton*>(QStringLiteral("serialQuickCommandButton"));
    QVERIFY(commandEdit != nullptr);
    QVERIFY(modeCombo != nullptr);
    QVERIFY(sendButton != nullptr);
    QVERIFY(!quickButtons.isEmpty());

    panel.setSendEnabled(false);

    QVERIFY(!commandEdit->isEnabled());
    QVERIFY(!modeCombo->isEnabled());
    QVERIFY(!sendButton->isEnabled());
    for (const QToolButton* button : quickButtons) {
        QVERIFY(!button->isEnabled());
    }
}

void SerialCommandPanelTest::reEnableSendKeepsButtonDisabledUntilCommandExists()
{
    SerialCommandPanel panel;
    auto* commandEdit = panel.findChild<QLineEdit*>(QStringLiteral("serialCommandEdit"));
    auto* sendButton = panel.findChild<QPushButton*>(QStringLiteral("serialCommandSendButton"));
    QVERIFY(commandEdit != nullptr);
    QVERIFY(sendButton != nullptr);

    panel.setSendEnabled(false);
    panel.setSendEnabled(true);

    QVERIFY(commandEdit->isEnabled());
    QVERIFY(!sendButton->isEnabled());

    commandEdit->setText(QStringLiteral("PING"));

    QVERIFY(sendButton->isEnabled());
}

void SerialCommandPanelTest::historyModelRejectsEmptyCommands()
{
    SerialCommandHistoryModel model;

    QVERIFY(!model.recordCommand(QStringLiteral("   "), QStringLiteral("ascii")));
    QCOMPARE(model.count(), 0);
    QVERIFY(model.isEmpty());
    QCOMPARE(model.commands(), QStringList());
}

void SerialCommandPanelTest::historyModelMovesDuplicateToFrontAndCountsUse()
{
    SerialCommandHistoryModel model;

    QVERIFY(model.recordCommand(QStringLiteral("AT+GMR"), QStringLiteral("ascii")));
    QVERIFY(model.recordCommand(QStringLiteral("PING"), QStringLiteral("ascii")));
    QVERIFY(model.recordCommand(QStringLiteral(" AT+GMR "), QStringLiteral("ascii")));

    QCOMPARE(model.count(), 2);
    QCOMPARE(model.commandAt(0), QStringLiteral("AT+GMR"));
    QCOMPARE(model.commandAt(1), QStringLiteral("PING"));
    QVERIFY(model.displayTextAt(0).contains(QStringLiteral("2x")));
}

void SerialCommandPanelTest::historyModelKeepsModeSpecificEntries()
{
    SerialCommandHistoryModel model;

    QVERIFY(model.recordCommand(QStringLiteral("01 03 00 00"), QStringLiteral("ascii")));
    QVERIFY(model.recordCommand(QStringLiteral("01 03 00 00"), QStringLiteral("hex")));

    QCOMPARE(model.count(), 2);
    QVERIFY(model.contains(QStringLiteral("01 03 00 00"), QStringLiteral("ascii")));
    QVERIFY(model.contains(QStringLiteral("01 03 00 00"), QStringLiteral("hex")));
    QCOMPARE(model.modeAt(0), QStringLiteral("hex"));
    QCOMPARE(model.modeAt(1), QStringLiteral("ascii"));
}

void SerialCommandPanelTest::historyModelTrimsOldestCommand()
{
    SerialCommandHistoryModel model(3);

    QVERIFY(model.recordCommand(QStringLiteral("CMD1"), QStringLiteral("ascii")));
    QVERIFY(model.recordCommand(QStringLiteral("CMD2"), QStringLiteral("ascii")));
    QVERIFY(model.recordCommand(QStringLiteral("CMD3"), QStringLiteral("ascii")));
    QVERIFY(model.recordCommand(QStringLiteral("CMD4"), QStringLiteral("ascii")));

    QCOMPARE(model.count(), 3);
    QCOMPARE(model.commands(), QStringList({QStringLiteral("CMD4"),
                                            QStringLiteral("CMD3"),
                                            QStringLiteral("CMD2")}));
    QVERIFY(!model.contains(QStringLiteral("CMD1"), QStringLiteral("ascii")));
}

void SerialCommandPanelTest::recordSentCommandUpdatesHistoryCombo()
{
    SerialCommandPanel panel;
    auto* historyCombo = panel.findChild<QComboBox*>(QStringLiteral("serialCommandHistoryCombo"));
    auto* clearButton = panel.findChild<QPushButton*>(QStringLiteral("serialHistoryClearButton"));
    QVERIFY(historyCombo != nullptr);
    QVERIFY(clearButton != nullptr);

    QCOMPARE(panel.historyCount(), 0);
    QVERIFY(!historyCombo->isEnabled());
    QVERIFY(!clearButton->isEnabled());

    panel.recordSentCommand(QStringLiteral("PING"), QStringLiteral("ascii"));

    QCOMPARE(panel.historyCount(), 1);
    QCOMPARE(panel.historyCommands(), QStringList({QStringLiteral("PING")}));
    QCOMPARE(historyCombo->count(), 1);
    QVERIFY(historyCombo->isEnabled());
    QVERIFY(clearButton->isEnabled());
    QVERIFY(historyCombo->itemText(0).contains(QStringLiteral("PING")));
}

void SerialCommandPanelTest::historyComboAppliesCommandAndMode()
{
    SerialCommandPanel panel;
    auto* commandEdit = panel.findChild<QLineEdit*>(QStringLiteral("serialCommandEdit"));
    auto* modeCombo = panel.findChild<QComboBox*>(QStringLiteral("serialCommandModeCombo"));
    auto* historyCombo = panel.findChild<QComboBox*>(QStringLiteral("serialCommandHistoryCombo"));
    auto* sendButton = panel.findChild<QPushButton*>(QStringLiteral("serialCommandSendButton"));
    QVERIFY(commandEdit != nullptr);
    QVERIFY(modeCombo != nullptr);
    QVERIFY(historyCombo != nullptr);
    QVERIFY(sendButton != nullptr);

    panel.recordSentCommand(QStringLiteral("AT+GMR"), QStringLiteral("ascii"));
    panel.recordSentCommand(QStringLiteral("01 03 00 00"), QStringLiteral("hex"));
    QCOMPARE(historyCombo->count(), 2);

    historyCombo->setCurrentIndex(0);
    QVERIFY(QMetaObject::invokeMethod(&panel, "applyHistoryCommand", Q_ARG(int, 0)));

    QCOMPARE(commandEdit->text(), QStringLiteral("01 03 00 00"));
    QCOMPARE(modeCombo->currentData().toString(), QStringLiteral("hex"));
    QVERIFY(sendButton->isEnabled());
}

void SerialCommandPanelTest::clearHistoryRemovesItemsWithoutChangingInput()
{
    SerialCommandPanel panel;
    auto* commandEdit = panel.findChild<QLineEdit*>(QStringLiteral("serialCommandEdit"));
    auto* historyCombo = panel.findChild<QComboBox*>(QStringLiteral("serialCommandHistoryCombo"));
    auto* clearButton = panel.findChild<QPushButton*>(QStringLiteral("serialHistoryClearButton"));
    QVERIFY(commandEdit != nullptr);
    QVERIFY(historyCombo != nullptr);
    QVERIFY(clearButton != nullptr);

    panel.recordSentCommand(QStringLiteral("PING"), QStringLiteral("ascii"));
    commandEdit->setText(QStringLiteral("AT+GMR"));
    QTest::mouseClick(clearButton, Qt::LeftButton);

    QCOMPARE(panel.historyCount(), 0);
    QCOMPARE(historyCombo->count(), 0);
    QCOMPARE(commandEdit->text(), QStringLiteral("AT+GMR"));
    QVERIFY(commandEdit->isEnabled());
    QVERIFY(!clearButton->isEnabled());
}

void SerialCommandPanelTest::sendIntentDoesNotRecordHistoryUntilConfirmed()
{
    SerialCommandPanel panel;
    auto* commandEdit = panel.findChild<QLineEdit*>(QStringLiteral("serialCommandEdit"));
    auto* sendButton = panel.findChild<QPushButton*>(QStringLiteral("serialCommandSendButton"));
    QVERIFY(commandEdit != nullptr);
    QVERIFY(sendButton != nullptr);

    commandEdit->setText(QStringLiteral("PING"));
    QTest::mouseClick(sendButton, Qt::LeftButton);

    QCOMPARE(panel.historyCount(), 0);
}

void SerialCommandPanelTest::confirmedSendRecordsTrimmedCommand()
{
    SerialCommandPanel panel;
    auto* commandEdit = panel.findChild<QLineEdit*>(QStringLiteral("serialCommandEdit"));
    auto* modeCombo = panel.findChild<QComboBox*>(QStringLiteral("serialCommandModeCombo"));
    QVERIFY(commandEdit != nullptr);
    QVERIFY(modeCombo != nullptr);

    const int protocolIndex = modeCombo->findData(QStringLiteral("protocol"));
    QVERIFY(protocolIndex >= 0);
    modeCombo->setCurrentIndex(protocolIndex);
    commandEdit->setText(QStringLiteral("  READ_ID  "));

    QSignalSpy spy(&panel, &SerialCommandPanel::sendRequested);
    QVERIFY(spy.isValid());
    QVERIFY(QMetaObject::invokeMethod(&panel, "emitSendRequested"));
    panel.confirmLastSentCommand();

    QCOMPARE(spy.count(), 1);
    QCOMPARE(panel.historyCount(), 1);
    QCOMPARE(panel.historyCommands(), QStringList({QStringLiteral("READ_ID")}));
}

void SerialCommandPanelTest::disablingSendDisablesHistorySelectionButKeepsClearAvailable()
{
    SerialCommandPanel panel;
    auto* historyCombo = panel.findChild<QComboBox*>(QStringLiteral("serialCommandHistoryCombo"));
    auto* clearButton = panel.findChild<QPushButton*>(QStringLiteral("serialHistoryClearButton"));
    QVERIFY(historyCombo != nullptr);
    QVERIFY(clearButton != nullptr);

    panel.recordSentCommand(QStringLiteral("PING"), QStringLiteral("ascii"));
    QVERIFY(historyCombo->isEnabled());
    QVERIFY(clearButton->isEnabled());

    panel.setSendEnabled(false);

    QVERIFY(!historyCombo->isEnabled());
    QVERIFY(clearButton->isEnabled());
}

void SerialCommandPanelTest::applyProfileCommandsRestoresDefaultModeAndHistory()
{
    SerialCommandPanel panel;
    auto* commandEdit = panel.findChild<QLineEdit*>(QStringLiteral("serialCommandEdit"));
    auto* modeCombo = panel.findChild<QComboBox*>(QStringLiteral("serialCommandModeCombo"));
    auto* historyCombo = panel.findChild<QComboBox*>(QStringLiteral("serialCommandHistoryCombo"));
    auto* sendButton = panel.findChild<QPushButton*>(QStringLiteral("serialCommandSendButton"));
    QVERIFY(commandEdit != nullptr);
    QVERIFY(modeCombo != nullptr);
    QVERIFY(historyCombo != nullptr);
    QVERIFY(sendButton != nullptr);

    const QVector<SerialProfileCommand> commands = {
        {QStringLiteral("Read Holding"), QStringLiteral("01 03 00 00 00 02"), QStringLiteral("hex")},
        {QStringLiteral("Read Version"), QStringLiteral("AT+GMR"), QStringLiteral("ascii")},
    };

    panel.recordSentCommand(QStringLiteral("OLD"), QStringLiteral("protocol"));
    panel.applyProfileCommands(commands, QStringLiteral("protocol"));

    QCOMPARE(panel.historyCount(), 2);
    QCOMPARE(panel.historyCommands(),
             QStringList({QStringLiteral("01 03 00 00 00 02"), QStringLiteral("AT+GMR")}));
    QCOMPARE(commandEdit->text(), QStringLiteral("01 03 00 00 00 02"));
    QCOMPARE(panel.sendMode(), QStringLiteral("hex"));
    QCOMPARE(modeCombo->currentData().toString(), QStringLiteral("hex"));
    QCOMPARE(historyCombo->count(), 2);
    QVERIFY(historyCombo->isEnabled());
    QVERIFY(sendButton->isEnabled());
}

void SerialCommandPanelTest::applyEmptyProfileCommandsClearsHistoryAndInput()
{
    SerialCommandPanel panel;
    auto* commandEdit = panel.findChild<QLineEdit*>(QStringLiteral("serialCommandEdit"));
    auto* modeCombo = panel.findChild<QComboBox*>(QStringLiteral("serialCommandModeCombo"));
    auto* historyCombo = panel.findChild<QComboBox*>(QStringLiteral("serialCommandHistoryCombo"));
    auto* clearButton = panel.findChild<QPushButton*>(QStringLiteral("serialHistoryClearButton"));
    auto* sendButton = panel.findChild<QPushButton*>(QStringLiteral("serialCommandSendButton"));
    QVERIFY(commandEdit != nullptr);
    QVERIFY(modeCombo != nullptr);
    QVERIFY(historyCombo != nullptr);
    QVERIFY(clearButton != nullptr);
    QVERIFY(sendButton != nullptr);

    panel.recordSentCommand(QStringLiteral("PING"), QStringLiteral("ascii"));
    commandEdit->setText(QStringLiteral("PING"));
    panel.applyProfileCommands({}, QStringLiteral("protocol"));

    QCOMPARE(panel.historyCount(), 0);
    QCOMPARE(historyCombo->count(), 0);
    QCOMPARE(commandEdit->text(), QString());
    QCOMPARE(panel.sendMode(), QStringLiteral("protocol"));
    QCOMPARE(modeCombo->currentData().toString(), QStringLiteral("protocol"));
    QVERIFY(!historyCombo->isEnabled());
    QVERIFY(!clearButton->isEnabled());
    QVERIFY(!sendButton->isEnabled());
}

QTEST_MAIN(SerialCommandPanelTest)
#include "test_serial_command_panel.moc"
