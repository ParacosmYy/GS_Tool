#include <QtTest/QtTest>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QToolButton>

#include "apps/serial_station/ui/SerialCommandPanel.h"

using serial_station::SerialCommandPanel;

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

QTEST_MAIN(SerialCommandPanelTest)
#include "test_serial_command_panel.moc"
