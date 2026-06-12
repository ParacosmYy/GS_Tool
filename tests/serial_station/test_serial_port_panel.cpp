#include <QtTest/QtTest>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QLabel>
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
    void closedStateEnablesConnectControls();
    void summaryLabelShowsDefaultUartConfig();
    void listedPortUsesItemDataInsteadOfDisplayText();
    void manualPortInputOverridesPreviousItemData();
    void baudRateChangeUpdatesSummary();
    void frameFormatChangesUpdateSummary();
    void flowControlChangeUpdatesSummary();
    void dtrRtsChangesUpdateSummary();
    void refreshButtonEmitsRefreshRequested();
    void connectSignalKeepsSelectedFrameFormat();
    void editableManualPortStillEmitsConnectRequest();
    void statusShowsClosedOpenAndErrorText();
    void summaryUpdatesWhenListedPortChanges();
    void customBaudTextIsAccepted();
    void openingStateDisablesConfigurationControls();
    void emptyManualPortKeepsMissingPortSummary();
    void refreshKeepsManualModeEditable();
    void invalidBaudTextShowsInvalidConfig();
    void errorStateCanReturnToClosedState();
};

namespace {

QComboBox* requireCombo(SerialPortPanel& panel, const QString& objectName)
{
    auto* combo = panel.findChild<QComboBox*>(objectName);
    Q_ASSERT(combo != nullptr);
    return combo;
}

QPushButton* requireButton(SerialPortPanel& panel, const QString& objectName)
{
    auto* button = panel.findChild<QPushButton*>(objectName);
    Q_ASSERT(button != nullptr);
    return button;
}

QCheckBox* requireCheckBox(SerialPortPanel& panel, const QString& objectName)
{
    auto* checkBox = panel.findChild<QCheckBox*>(objectName);
    Q_ASSERT(checkBox != nullptr);
    return checkBox;
}

QLabel* requireLabel(SerialPortPanel& panel, const QString& objectName)
{
    auto* label = panel.findChild<QLabel*>(objectName);
    Q_ASSERT(label != nullptr);
    return label;
}

void forceSingleListedPort(SerialPortPanel& panel,
                           const QString& displayText,
                           const QString& portName)
{
    auto* portCombo = requireCombo(panel, QStringLiteral("serialPortCombo"));
    portCombo->clear();
    portCombo->setEditable(true);
    portCombo->addItem(displayText, portName);
    portCombo->setCurrentIndex(0);
}

void setComboByText(SerialPortPanel& panel,
                    const QString& objectName,
                    const QString& text)
{
    auto* combo = requireCombo(panel, objectName);
    const int index = combo->findText(text);
    QVERIFY(index >= 0);
    combo->setCurrentIndex(index);
}

void setComboByData(SerialPortPanel& panel,
                    const QString& objectName,
                    int data)
{
    auto* combo = requireCombo(panel, objectName);
    const int index = combo->findData(data);
    QVERIFY(index >= 0);
    combo->setCurrentIndex(index);
}

QString summaryText(SerialPortPanel& panel)
{
    return requireLabel(panel, QStringLiteral("serialUartSummaryLabel"))->text();
}

} // namespace

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
    auto* disconnectButton = requireButton(panel, QStringLiteral("serialDisconnectButton"));

    panel.setSessionState(SerialSessionState::Open);

    QSignalSpy spy(&panel, &SerialPortPanel::disconnectRequested);
    QVERIFY(spy.isValid());
    QTest::mouseClick(disconnectButton, Qt::LeftButton);

    QCOMPARE(spy.count(), 1);
}

void SerialPortPanelTest::openStateDisablesConnectControls()
{
    SerialPortPanel panel;
    auto* portCombo = requireCombo(panel, QStringLiteral("serialPortCombo"));
    auto* baudCombo = requireCombo(panel, QStringLiteral("serialBaudCombo"));
    auto* dataBitsCombo = requireCombo(panel, QStringLiteral("serialDataBitsCombo"));
    auto* parityCombo = requireCombo(panel, QStringLiteral("serialParityCombo"));
    auto* stopBitsCombo = requireCombo(panel, QStringLiteral("serialStopBitsCombo"));
    auto* flowControlCombo = requireCombo(panel, QStringLiteral("serialFlowControlCombo"));
    auto* dtrCheck = requireCheckBox(panel, QStringLiteral("serialDtrCheck"));
    auto* rtsCheck = requireCheckBox(panel, QStringLiteral("serialRtsCheck"));
    auto* connectButton = requireButton(panel, QStringLiteral("serialConnectButton"));
    auto* disconnectButton = requireButton(panel, QStringLiteral("serialDisconnectButton"));

    panel.setSessionState(SerialSessionState::Open);

    QVERIFY(!portCombo->isEnabled());
    QVERIFY(!baudCombo->isEnabled());
    QVERIFY(!dataBitsCombo->isEnabled());
    QVERIFY(!parityCombo->isEnabled());
    QVERIFY(!stopBitsCombo->isEnabled());
    QVERIFY(!flowControlCombo->isEnabled());
    QVERIFY(!dtrCheck->isEnabled());
    QVERIFY(!rtsCheck->isEnabled());
    QVERIFY(!connectButton->isEnabled());
    QVERIFY(disconnectButton->isEnabled());
}

void SerialPortPanelTest::closedStateEnablesConnectControls()
{
    SerialPortPanel panel;
    panel.setSessionState(SerialSessionState::Open);
    panel.setSessionState(SerialSessionState::Closed);

    QVERIFY(requireCombo(panel, QStringLiteral("serialPortCombo"))->isEnabled());
    QVERIFY(requireCombo(panel, QStringLiteral("serialBaudCombo"))->isEnabled());
    QVERIFY(requireCombo(panel, QStringLiteral("serialDataBitsCombo"))->isEnabled());
    QVERIFY(requireCombo(panel, QStringLiteral("serialParityCombo"))->isEnabled());
    QVERIFY(requireCombo(panel, QStringLiteral("serialStopBitsCombo"))->isEnabled());
    QVERIFY(requireCombo(panel, QStringLiteral("serialFlowControlCombo"))->isEnabled());
    QVERIFY(requireCheckBox(panel, QStringLiteral("serialDtrCheck"))->isEnabled());
    QVERIFY(requireCheckBox(panel, QStringLiteral("serialRtsCheck"))->isEnabled());
    QVERIFY(requireButton(panel, QStringLiteral("serialConnectButton"))->isEnabled());
}

void SerialPortPanelTest::summaryLabelShowsDefaultUartConfig()
{
    SerialPortPanel panel;
    auto* portCombo = requireCombo(panel, QStringLiteral("serialPortCombo"));
    portCombo->setEditable(true);
    portCombo->setEditText(QStringLiteral("COM_SUMMARY"));

    const QString summary = summaryText(panel);

    QVERIFY(summary.startsWith(QStringLiteral("UART:")));
    QVERIFY(summary.contains(QStringLiteral("COM_SUMMARY")));
    QVERIFY(summary.contains(QStringLiteral("115200")));
    QVERIFY(summary.contains(QStringLiteral("8N1")));
    QVERIFY(summary.contains(QStringLiteral("无流控")));
    QVERIFY(summary.contains(QStringLiteral("DTR=off")));
    QVERIFY(summary.contains(QStringLiteral("RTS=off")));
}

void SerialPortPanelTest::listedPortUsesItemDataInsteadOfDisplayText()
{
    SerialPortPanel panel;
    forceSingleListedPort(panel,
                          QStringLiteral("COM7 - USB Serial Device - VID:1A86 PID:7523"),
                          QStringLiteral("COM7"));

    const SerialPortConfig config = panel.currentConfig();

    QCOMPARE(config.portName, QStringLiteral("COM7"));
    QVERIFY(summaryText(panel).contains(QStringLiteral("COM7")));
    QVERIFY(!config.portName.contains(QStringLiteral("USB Serial Device")));
}

void SerialPortPanelTest::manualPortInputOverridesPreviousItemData()
{
    SerialPortPanel panel;
    forceSingleListedPort(panel,
                          QStringLiteral("COM8 - Listed Device"),
                          QStringLiteral("COM8"));

    auto* portCombo = requireCombo(panel, QStringLiteral("serialPortCombo"));
    portCombo->setEditText(QStringLiteral("COM_MANUAL"));

    const SerialPortConfig config = panel.currentConfig();

    QCOMPARE(config.portName, QStringLiteral("COM_MANUAL"));
    QVERIFY(summaryText(panel).contains(QStringLiteral("COM_MANUAL")));
}

void SerialPortPanelTest::baudRateChangeUpdatesSummary()
{
    SerialPortPanel panel;
    auto* portCombo = requireCombo(panel, QStringLiteral("serialPortCombo"));
    auto* baudCombo = requireCombo(panel, QStringLiteral("serialBaudCombo"));
    portCombo->setEditable(true);
    portCombo->setEditText(QStringLiteral("COM_BAUD"));

    baudCombo->setCurrentText(QStringLiteral("921600"));

    QCOMPARE(panel.currentConfig().baudRate, 921600);
    QVERIFY(summaryText(panel).contains(QStringLiteral("921600")));
}

void SerialPortPanelTest::frameFormatChangesUpdateSummary()
{
    SerialPortPanel panel;
    auto* portCombo = requireCombo(panel, QStringLiteral("serialPortCombo"));
    portCombo->setEditable(true);
    portCombo->setEditText(QStringLiteral("COM_FRAME"));

    setComboByText(panel, QStringLiteral("serialDataBitsCombo"), QStringLiteral("7"));
    setComboByText(panel, QStringLiteral("serialParityCombo"), QStringLiteral("偶校验"));
    setComboByText(panel, QStringLiteral("serialStopBitsCombo"), QStringLiteral("2"));

    const SerialPortConfig config = panel.currentConfig();
    QCOMPARE(config.dataBits, QSerialPort::Data7);
    QCOMPARE(config.parity, QSerialPort::EvenParity);
    QCOMPARE(config.stopBits, QSerialPort::TwoStop);
    QVERIFY(summaryText(panel).contains(QStringLiteral("7E2")));
}

void SerialPortPanelTest::flowControlChangeUpdatesSummary()
{
    SerialPortPanel panel;
    auto* portCombo = requireCombo(panel, QStringLiteral("serialPortCombo"));
    portCombo->setEditable(true);
    portCombo->setEditText(QStringLiteral("COM_FLOW"));

    setComboByData(panel,
                   QStringLiteral("serialFlowControlCombo"),
                   static_cast<int>(QSerialPort::HardwareControl));

    QCOMPARE(panel.currentConfig().flowControl, QSerialPort::HardwareControl);
    QVERIFY(summaryText(panel).contains(QStringLiteral("硬件流控")));
}

void SerialPortPanelTest::dtrRtsChangesUpdateSummary()
{
    SerialPortPanel panel;
    auto* portCombo = requireCombo(panel, QStringLiteral("serialPortCombo"));
    portCombo->setEditable(true);
    portCombo->setEditText(QStringLiteral("COM_LINES"));

    requireCheckBox(panel, QStringLiteral("serialDtrCheck"))->setChecked(true);
    requireCheckBox(panel, QStringLiteral("serialRtsCheck"))->setChecked(true);

    const SerialPortConfig config = panel.currentConfig();
    QVERIFY(config.dtrEnabled);
    QVERIFY(config.rtsEnabled);
    QVERIFY(summaryText(panel).contains(QStringLiteral("DTR=on")));
    QVERIFY(summaryText(panel).contains(QStringLiteral("RTS=on")));
}

void SerialPortPanelTest::refreshButtonEmitsRefreshRequested()
{
    SerialPortPanel panel;
    auto* refreshButton = requireButton(panel, QStringLiteral("serialRefreshButton"));

    QSignalSpy spy(&panel, &SerialPortPanel::refreshRequested);
    QVERIFY(spy.isValid());

    QTest::mouseClick(refreshButton, Qt::LeftButton);

    QCOMPARE(spy.count(), 1);
}

void SerialPortPanelTest::connectSignalKeepsSelectedFrameFormat()
{
    qRegisterMetaType<SerialPortConfig>("SerialPortConfig");

    SerialPortPanel panel;
    forceSingleListedPort(panel,
                          QStringLiteral("COM9 - USB UART"),
                          QStringLiteral("COM9"));
    setComboByText(panel, QStringLiteral("serialDataBitsCombo"), QStringLiteral("7"));
    setComboByText(panel, QStringLiteral("serialParityCombo"), QStringLiteral("奇校验"));
    setComboByText(panel, QStringLiteral("serialStopBitsCombo"), QStringLiteral("2"));
    setComboByData(panel,
                   QStringLiteral("serialFlowControlCombo"),
                   static_cast<int>(QSerialPort::SoftwareControl));
    requireCheckBox(panel, QStringLiteral("serialDtrCheck"))->setChecked(true);

    QSignalSpy spy(&panel, &SerialPortPanel::connectRequested);
    QVERIFY(spy.isValid());
    QTest::mouseClick(requireButton(panel, QStringLiteral("serialConnectButton")), Qt::LeftButton);

    QCOMPARE(spy.count(), 1);
    const SerialPortConfig config = qvariant_cast<SerialPortConfig>(spy.takeFirst().at(0));
    QCOMPARE(config.portName, QStringLiteral("COM9"));
    QCOMPARE(config.dataBits, QSerialPort::Data7);
    QCOMPARE(config.parity, QSerialPort::OddParity);
    QCOMPARE(config.stopBits, QSerialPort::TwoStop);
    QCOMPARE(config.flowControl, QSerialPort::SoftwareControl);
    QVERIFY(config.dtrEnabled);
    QVERIFY(!config.rtsEnabled);
}

void SerialPortPanelTest::editableManualPortStillEmitsConnectRequest()
{
    qRegisterMetaType<SerialPortConfig>("SerialPortConfig");

    SerialPortPanel panel;
    auto* portCombo = requireCombo(panel, QStringLiteral("serialPortCombo"));
    portCombo->clear();
    portCombo->setEditable(true);
    portCombo->setEditText(QStringLiteral("COM_TYPED"));

    QSignalSpy spy(&panel, &SerialPortPanel::connectRequested);
    QVERIFY(spy.isValid());
    QTest::mouseClick(requireButton(panel, QStringLiteral("serialConnectButton")), Qt::LeftButton);

    QCOMPARE(spy.count(), 1);
    const SerialPortConfig config = qvariant_cast<SerialPortConfig>(spy.takeFirst().at(0));
    QCOMPARE(config.portName, QStringLiteral("COM_TYPED"));
}

void SerialPortPanelTest::statusShowsClosedOpenAndErrorText()
{
    SerialPortPanel panel;
    auto* statusLabel = requireLabel(panel, QStringLiteral("serialPortStatusLabel"));

    panel.setSessionState(SerialSessionState::Closed);
    QCOMPARE(statusLabel->text(), QStringLiteral("未连接"));

    panel.setSessionState(SerialSessionState::Open);
    QCOMPARE(statusLabel->text(), QStringLiteral("已连接"));

    panel.setErrorMessage(QStringLiteral("打开 COM1 失败"));
    QCOMPARE(statusLabel->text(), QStringLiteral("打开 COM1 失败"));
}

void SerialPortPanelTest::summaryUpdatesWhenListedPortChanges()
{
    SerialPortPanel panel;
    auto* portCombo = requireCombo(panel, QStringLiteral("serialPortCombo"));
    portCombo->clear();
    portCombo->setEditable(true);
    portCombo->addItem(QStringLiteral("COM10 - USB UART A"), QStringLiteral("COM10"));
    portCombo->addItem(QStringLiteral("COM11 - USB UART B"), QStringLiteral("COM11"));

    portCombo->setCurrentIndex(0);
    QVERIFY(summaryText(panel).contains(QStringLiteral("COM10")));
    QCOMPARE(panel.currentConfig().portName, QStringLiteral("COM10"));

    portCombo->setCurrentIndex(1);
    QVERIFY(summaryText(panel).contains(QStringLiteral("COM11")));
    QCOMPARE(panel.currentConfig().portName, QStringLiteral("COM11"));
}

void SerialPortPanelTest::customBaudTextIsAccepted()
{
    SerialPortPanel panel;
    auto* portCombo = requireCombo(panel, QStringLiteral("serialPortCombo"));
    auto* baudCombo = requireCombo(panel, QStringLiteral("serialBaudCombo"));
    portCombo->setEditable(true);
    portCombo->setEditText(QStringLiteral("COM_CUSTOM_BAUD"));

    baudCombo->setEditText(QStringLiteral("250000"));

    QCOMPARE(panel.currentConfig().baudRate, 250000);
    QVERIFY(summaryText(panel).contains(QStringLiteral("250000")));
    QVERIFY(panel.currentConfig().isValid());
}

void SerialPortPanelTest::openingStateDisablesConfigurationControls()
{
    SerialPortPanel panel;

    panel.setSessionState(SerialSessionState::Opening);

    QVERIFY(!requireCombo(panel, QStringLiteral("serialPortCombo"))->isEnabled());
    QVERIFY(!requireCombo(panel, QStringLiteral("serialBaudCombo"))->isEnabled());
    QVERIFY(!requireCombo(panel, QStringLiteral("serialDataBitsCombo"))->isEnabled());
    QVERIFY(!requireCombo(panel, QStringLiteral("serialParityCombo"))->isEnabled());
    QVERIFY(!requireCombo(panel, QStringLiteral("serialStopBitsCombo"))->isEnabled());
    QVERIFY(!requireCombo(panel, QStringLiteral("serialFlowControlCombo"))->isEnabled());
    QVERIFY(!requireCheckBox(panel, QStringLiteral("serialDtrCheck"))->isEnabled());
    QVERIFY(!requireCheckBox(panel, QStringLiteral("serialRtsCheck"))->isEnabled());
    QVERIFY(!requireButton(panel, QStringLiteral("serialConnectButton"))->isEnabled());
    QVERIFY(requireButton(panel, QStringLiteral("serialDisconnectButton"))->isEnabled());
}

void SerialPortPanelTest::emptyManualPortKeepsMissingPortSummary()
{
    SerialPortPanel panel;
    auto* portCombo = requireCombo(panel, QStringLiteral("serialPortCombo"));
    portCombo->clear();
    portCombo->setEditable(true);
    portCombo->setEditText(QStringLiteral("   "));

    QVERIFY(summaryText(panel).contains(QStringLiteral("<未选择端口>")));
    QVERIFY(!panel.currentConfig().isValid());
    QCOMPARE(panel.currentConfig().validationError(), QStringLiteral("串口端口名为空"));
}

void SerialPortPanelTest::refreshKeepsManualModeEditable()
{
    SerialPortPanel panel;
    auto* portCombo = requireCombo(panel, QStringLiteral("serialPortCombo"));
    auto* refreshButton = requireButton(panel, QStringLiteral("serialRefreshButton"));

    portCombo->clear();
    portCombo->setEditable(true);
    portCombo->setEditText(QStringLiteral("COM_BEFORE_REFRESH"));

    QTest::mouseClick(refreshButton, Qt::LeftButton);

    QVERIFY(portCombo->isEditable());
    QVERIFY(requireLabel(panel, QStringLiteral("serialUartSummaryLabel"))->isVisible()
            || !requireLabel(panel, QStringLiteral("serialUartSummaryLabel"))->text().isEmpty());
}

void SerialPortPanelTest::invalidBaudTextShowsInvalidConfig()
{
    SerialPortPanel panel;
    auto* portCombo = requireCombo(panel, QStringLiteral("serialPortCombo"));
    auto* baudCombo = requireCombo(panel, QStringLiteral("serialBaudCombo"));
    portCombo->setEditable(true);
    portCombo->setEditText(QStringLiteral("COM_BAD_BAUD"));

    baudCombo->setEditText(QStringLiteral("bad"));

    QCOMPARE(panel.currentConfig().baudRate, 0);
    QVERIFY(!panel.currentConfig().isValid());
    QCOMPARE(panel.currentConfig().validationError(),
             QStringLiteral("串口波特率必须大于 0"));
    QVERIFY(summaryText(panel).contains(QStringLiteral("0")));
}

void SerialPortPanelTest::errorStateCanReturnToClosedState()
{
    SerialPortPanel panel;
    auto* statusLabel = requireLabel(panel, QStringLiteral("serialPortStatusLabel"));

    panel.setSessionState(SerialSessionState::Error);
    QCOMPARE(statusLabel->text(), QStringLiteral("连接错误"));
    QVERIFY(requireButton(panel, QStringLiteral("serialConnectButton"))->isEnabled());

    panel.setSessionState(SerialSessionState::Closed);
    QCOMPARE(statusLabel->text(), QStringLiteral("未连接"));
    QVERIFY(requireButton(panel, QStringLiteral("serialConnectButton"))->isEnabled());
    QVERIFY(!requireButton(panel, QStringLiteral("serialDisconnectButton"))->isEnabled());
}

QTEST_MAIN(SerialPortPanelTest)
#include "test_serial_port_panel.moc"
