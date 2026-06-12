#include <QtTest/QtTest>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QPushButton>

#include "apps/serial_station/ui/SerialPortPanel.h"

using serial_station::SerialPortConfig;
using serial_station::SerialPortPanel;
using serial_station::SerialSessionState;

Q_DECLARE_METATYPE(SerialPortConfig)

class SerialPortPanelTest : public QObject {
    Q_OBJECT

private slots:
    void defaultConfigUses1152008N1();
    void connectButtonEmitsCurrentConfig();
    void disconnectButtonEmitsRequest();
    void openStateDisablesConnectControls();
};

void SerialPortPanelTest::defaultConfigUses1152008N1()
{
    SerialPortPanel panel;
    auto* portCombo = panel.findChild<QComboBox*>(QStringLiteral("serialPortCombo"));
    QVERIFY(portCombo != nullptr);
    portCombo->setEditable(true);
    portCombo->setEditText(QStringLiteral("COM_TEST"));

    const SerialPortConfig config = panel.currentConfig();
    QCOMPARE(config.portName, QStringLiteral("COM_TEST"));
    QCOMPARE(config.baudRate, 115200);
    QCOMPARE(config.dataBits, QSerialPort::Data8);
    QCOMPARE(config.parity, QSerialPort::NoParity);
    QCOMPARE(config.stopBits, QSerialPort::OneStop);
    QCOMPARE(config.flowControl, QSerialPort::NoFlowControl);
    QCOMPARE(config.dtrEnabled, false);
    QCOMPARE(config.rtsEnabled, false);
}

void SerialPortPanelTest::connectButtonEmitsCurrentConfig()
{
    qRegisterMetaType<SerialPortConfig>("SerialPortConfig");

    SerialPortPanel panel;
    auto* portCombo = panel.findChild<QComboBox*>(QStringLiteral("serialPortCombo"));
    auto* connectButton = panel.findChild<QPushButton*>(QStringLiteral("serialConnectButton"));
    QVERIFY(portCombo != nullptr);
    QVERIFY(connectButton != nullptr);

    portCombo->setEditable(true);
    portCombo->setEditText(QStringLiteral("COM_SIGNAL"));

    QSignalSpy spy(&panel, &SerialPortPanel::connectRequested);
    QVERIFY(spy.isValid());
    QTest::mouseClick(connectButton, Qt::LeftButton);

    QCOMPARE(spy.count(), 1);
    const SerialPortConfig config = qvariant_cast<SerialPortConfig>(spy.takeFirst().at(0));
    QCOMPARE(config.portName, QStringLiteral("COM_SIGNAL"));
    QCOMPARE(config.baudRate, 115200);
}

void SerialPortPanelTest::disconnectButtonEmitsRequest()
{
    SerialPortPanel panel;
    auto* disconnectButton = panel.findChild<QPushButton*>(QStringLiteral("serialDisconnectButton"));
    QVERIFY(disconnectButton != nullptr);

    panel.setSessionState(SerialSessionState::Open);

    QSignalSpy spy(&panel, &SerialPortPanel::disconnectRequested);
    QVERIFY(spy.isValid());
    QTest::mouseClick(disconnectButton, Qt::LeftButton);

    QCOMPARE(spy.count(), 1);
}

void SerialPortPanelTest::openStateDisablesConnectControls()
{
    SerialPortPanel panel;
    auto* portCombo = panel.findChild<QComboBox*>(QStringLiteral("serialPortCombo"));
    auto* connectButton = panel.findChild<QPushButton*>(QStringLiteral("serialConnectButton"));
    auto* disconnectButton = panel.findChild<QPushButton*>(QStringLiteral("serialDisconnectButton"));
    QVERIFY(portCombo != nullptr);
    QVERIFY(connectButton != nullptr);
    QVERIFY(disconnectButton != nullptr);

    panel.setSessionState(SerialSessionState::Open);

    QVERIFY(!portCombo->isEnabled());
    QVERIFY(!connectButton->isEnabled());
    QVERIFY(disconnectButton->isEnabled());
}

QTEST_MAIN(SerialPortPanelTest)
#include "test_serial_port_panel.moc"
