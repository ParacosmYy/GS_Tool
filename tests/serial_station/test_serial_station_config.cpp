#include <QtTest/QtTest>

#include "apps/serial_station/SerialStationConfig.h"

using serial_station::SerialPortConfig;
using serial_station::SerialStationConfig;

class SerialStationConfigTest : public QObject {
    Q_OBJECT

private slots:
    void defaultPortConfigUses1152008N1();
    void normalizedPortNameTrimsWhitespace_data();
    void normalizedPortNameTrimsWhitespace();
    void normalizedConfigKeepsLineAndFrameSettings();
    void validConfigAcceptsCommonFrameFormats_data();
    void validConfigAcceptsCommonFrameFormats();
    void validationRejectsMissingPort_data();
    void validationRejectsMissingPort();
    void validationRejectsInvalidBaudRate_data();
    void validationRejectsInvalidBaudRate();
    void validationRejectsUnknownEnumValues_data();
    void validationRejectsUnknownEnumValues();
    void summaryUsesStableCompactFormat_data();
    void summaryUsesStableCompactFormat();
    void summaryMarksMissingPort();
    void summaryIncludesControlLineStates_data();
    void summaryIncludesControlLineStates();
    void reconnectRequiresPositiveInterval();
    void reconnectDisabledWhenFlagIsFalse();
    void reconnectAllowsCustomInterval();
};

void SerialStationConfigTest::defaultPortConfigUses1152008N1()
{
    SerialPortConfig config;
    config.portName = QStringLiteral("COM1");

    QVERIFY(config.isValid());
    QCOMPARE(config.normalizedPortName(), QStringLiteral("COM1"));
    QCOMPARE(config.validationError(), QString());
    QCOMPARE(config.baudRate, 115200);
    QCOMPARE(config.dataBits, QSerialPort::Data8);
    QCOMPARE(config.parity, QSerialPort::NoParity);
    QCOMPARE(config.stopBits, QSerialPort::OneStop);
    QCOMPARE(config.flowControl, QSerialPort::NoFlowControl);
    QCOMPARE(config.summary(), QStringLiteral("COM1 115200 8N1 无流控 DTR=off RTS=off"));
}

void SerialStationConfigTest::normalizedPortNameTrimsWhitespace_data()
{
    QTest::addColumn<QString>("rawPort");
    QTest::addColumn<QString>("normalizedPort");

    QTest::newRow("leading") << QStringLiteral("  COM3") << QStringLiteral("COM3");
    QTest::newRow("trailing") << QStringLiteral("COM4  ") << QStringLiteral("COM4");
    QTest::newRow("both") << QStringLiteral("  COM5  ") << QStringLiteral("COM5");
    QTest::newRow("unix") << QStringLiteral("  /dev/ttyUSB0  ") << QStringLiteral("/dev/ttyUSB0");
    QTest::newRow("bluetooth") << QStringLiteral("\tCOM_BT\r\n") << QStringLiteral("COM_BT");
}

void SerialStationConfigTest::normalizedPortNameTrimsWhitespace()
{
    QFETCH(QString, rawPort);
    QFETCH(QString, normalizedPort);

    SerialPortConfig config;
    config.portName = rawPort;

    QCOMPARE(config.normalizedPortName(), normalizedPort);
    QCOMPARE(config.normalized().portName, normalizedPort);
}

void SerialStationConfigTest::normalizedConfigKeepsLineAndFrameSettings()
{
    SerialPortConfig config;
    config.portName = QStringLiteral("  COM9 ");
    config.baudRate = 921600;
    config.dataBits = QSerialPort::Data7;
    config.parity = QSerialPort::EvenParity;
    config.stopBits = QSerialPort::TwoStop;
    config.flowControl = QSerialPort::HardwareControl;
    config.dtrEnabled = true;
    config.rtsEnabled = true;

    const SerialPortConfig normalized = config.normalized();

    QCOMPARE(normalized.portName, QStringLiteral("COM9"));
    QCOMPARE(normalized.baudRate, 921600);
    QCOMPARE(normalized.dataBits, QSerialPort::Data7);
    QCOMPARE(normalized.parity, QSerialPort::EvenParity);
    QCOMPARE(normalized.stopBits, QSerialPort::TwoStop);
    QCOMPARE(normalized.flowControl, QSerialPort::HardwareControl);
    QVERIFY(normalized.dtrEnabled);
    QVERIFY(normalized.rtsEnabled);
}

void SerialStationConfigTest::validConfigAcceptsCommonFrameFormats_data()
{
    QTest::addColumn<QSerialPort::DataBits>("dataBits");
    QTest::addColumn<QSerialPort::Parity>("parity");
    QTest::addColumn<QSerialPort::StopBits>("stopBits");
    QTest::addColumn<QSerialPort::FlowControl>("flowControl");

    QTest::newRow("8n1-none") << QSerialPort::Data8 << QSerialPort::NoParity
                              << QSerialPort::OneStop << QSerialPort::NoFlowControl;
    QTest::newRow("8e1-hardware") << QSerialPort::Data8 << QSerialPort::EvenParity
                                  << QSerialPort::OneStop << QSerialPort::HardwareControl;
    QTest::newRow("8o2-software") << QSerialPort::Data8 << QSerialPort::OddParity
                                  << QSerialPort::TwoStop << QSerialPort::SoftwareControl;
    QTest::newRow("7e1-none") << QSerialPort::Data7 << QSerialPort::EvenParity
                              << QSerialPort::OneStop << QSerialPort::NoFlowControl;
    QTest::newRow("7o2-none") << QSerialPort::Data7 << QSerialPort::OddParity
                              << QSerialPort::TwoStop << QSerialPort::NoFlowControl;
    QTest::newRow("6m1-none") << QSerialPort::Data6 << QSerialPort::MarkParity
                              << QSerialPort::OneStop << QSerialPort::NoFlowControl;
    QTest::newRow("5s1.5-none") << QSerialPort::Data5 << QSerialPort::SpaceParity
                                << QSerialPort::OneAndHalfStop << QSerialPort::NoFlowControl;
}

void SerialStationConfigTest::validConfigAcceptsCommonFrameFormats()
{
    QFETCH(QSerialPort::DataBits, dataBits);
    QFETCH(QSerialPort::Parity, parity);
    QFETCH(QSerialPort::StopBits, stopBits);
    QFETCH(QSerialPort::FlowControl, flowControl);

    SerialPortConfig config;
    config.portName = QStringLiteral("COM_VALID");
    config.baudRate = 115200;
    config.dataBits = dataBits;
    config.parity = parity;
    config.stopBits = stopBits;
    config.flowControl = flowControl;

    QVERIFY(config.isValid());
    QCOMPARE(config.validationError(), QString());
}

void SerialStationConfigTest::validationRejectsMissingPort_data()
{
    QTest::addColumn<QString>("portName");

    QTest::newRow("empty") << QString();
    QTest::newRow("spaces") << QStringLiteral("   ");
    QTest::newRow("tabs") << QStringLiteral("\t\t");
    QTest::newRow("newline") << QStringLiteral("\r\n");
}

void SerialStationConfigTest::validationRejectsMissingPort()
{
    QFETCH(QString, portName);

    SerialPortConfig config;
    config.portName = portName;

    QVERIFY(!config.isValid());
    QCOMPARE(config.validationError(), QStringLiteral("串口端口名为空"));
}

void SerialStationConfigTest::validationRejectsInvalidBaudRate_data()
{
    QTest::addColumn<int>("baudRate");

    QTest::newRow("zero") << 0;
    QTest::newRow("negative") << -1;
    QTest::newRow("large-negative") << -115200;
}

void SerialStationConfigTest::validationRejectsInvalidBaudRate()
{
    QFETCH(int, baudRate);

    SerialPortConfig config;
    config.portName = QStringLiteral("COM_BAD_BAUD");
    config.baudRate = baudRate;

    QVERIFY(!config.isValid());
    QCOMPARE(config.validationError(), QStringLiteral("串口波特率必须大于 0"));
}

void SerialStationConfigTest::validationRejectsUnknownEnumValues_data()
{
    QTest::addColumn<QSerialPort::DataBits>("dataBits");
    QTest::addColumn<QSerialPort::Parity>("parity");
    QTest::addColumn<QSerialPort::StopBits>("stopBits");
    QTest::addColumn<QSerialPort::FlowControl>("flowControl");
    QTest::addColumn<QString>("error");

    QTest::newRow("data-bits") << static_cast<QSerialPort::DataBits>(-1) << QSerialPort::NoParity
                               << QSerialPort::OneStop << QSerialPort::NoFlowControl
                               << QStringLiteral("串口数据位不受支持");
    QTest::newRow("parity") << QSerialPort::Data8 << static_cast<QSerialPort::Parity>(-1)
                            << QSerialPort::OneStop << QSerialPort::NoFlowControl
                            << QStringLiteral("串口校验位不受支持");
    QTest::newRow("stop-bits") << QSerialPort::Data8 << QSerialPort::NoParity
                               << static_cast<QSerialPort::StopBits>(-1) << QSerialPort::NoFlowControl
                               << QStringLiteral("串口停止位不受支持");
    QTest::newRow("flow-control") << QSerialPort::Data8 << QSerialPort::NoParity
                                  << QSerialPort::OneStop << static_cast<QSerialPort::FlowControl>(-1)
                                  << QStringLiteral("串口流控不受支持");
}

void SerialStationConfigTest::validationRejectsUnknownEnumValues()
{
    QFETCH(QSerialPort::DataBits, dataBits);
    QFETCH(QSerialPort::Parity, parity);
    QFETCH(QSerialPort::StopBits, stopBits);
    QFETCH(QSerialPort::FlowControl, flowControl);
    QFETCH(QString, error);

    SerialPortConfig config;
    config.portName = QStringLiteral("COM_ENUM");
    config.dataBits = dataBits;
    config.parity = parity;
    config.stopBits = stopBits;
    config.flowControl = flowControl;

    QVERIFY(!config.isValid());
    QCOMPARE(config.validationError(), error);
}

void SerialStationConfigTest::summaryUsesStableCompactFormat_data()
{
    QTest::addColumn<QSerialPort::DataBits>("dataBits");
    QTest::addColumn<QSerialPort::Parity>("parity");
    QTest::addColumn<QSerialPort::StopBits>("stopBits");
    QTest::addColumn<QSerialPort::FlowControl>("flowControl");
    QTest::addColumn<QString>("summary");

    QTest::newRow("8n1") << QSerialPort::Data8 << QSerialPort::NoParity
                         << QSerialPort::OneStop << QSerialPort::NoFlowControl
                         << QStringLiteral("COM_SUM 115200 8N1 无流控 DTR=off RTS=off");
    QTest::newRow("8e2") << QSerialPort::Data8 << QSerialPort::EvenParity
                         << QSerialPort::TwoStop << QSerialPort::NoFlowControl
                         << QStringLiteral("COM_SUM 115200 8E2 无流控 DTR=off RTS=off");
    QTest::newRow("7o1") << QSerialPort::Data7 << QSerialPort::OddParity
                         << QSerialPort::OneStop << QSerialPort::NoFlowControl
                         << QStringLiteral("COM_SUM 115200 7O1 无流控 DTR=off RTS=off");
    QTest::newRow("6m1") << QSerialPort::Data6 << QSerialPort::MarkParity
                         << QSerialPort::OneStop << QSerialPort::HardwareControl
                         << QStringLiteral("COM_SUM 115200 6M1 硬件流控 DTR=off RTS=off");
    QTest::newRow("5s1.5") << QSerialPort::Data5 << QSerialPort::SpaceParity
                           << QSerialPort::OneAndHalfStop << QSerialPort::SoftwareControl
                           << QStringLiteral("COM_SUM 115200 5S1.5 软件流控 DTR=off RTS=off");
}

void SerialStationConfigTest::summaryUsesStableCompactFormat()
{
    QFETCH(QSerialPort::DataBits, dataBits);
    QFETCH(QSerialPort::Parity, parity);
    QFETCH(QSerialPort::StopBits, stopBits);
    QFETCH(QSerialPort::FlowControl, flowControl);
    QFETCH(QString, summary);

    SerialPortConfig config;
    config.portName = QStringLiteral("COM_SUM");
    config.dataBits = dataBits;
    config.parity = parity;
    config.stopBits = stopBits;
    config.flowControl = flowControl;

    QCOMPARE(config.summary(), summary);
}

void SerialStationConfigTest::summaryMarksMissingPort()
{
    SerialPortConfig config;
    config.portName = QStringLiteral("   ");

    QCOMPARE(config.summary(), QStringLiteral("<未选择端口> 115200 8N1 无流控 DTR=off RTS=off"));
}

void SerialStationConfigTest::summaryIncludesControlLineStates_data()
{
    QTest::addColumn<bool>("dtrEnabled");
    QTest::addColumn<bool>("rtsEnabled");
    QTest::addColumn<QString>("suffix");

    QTest::newRow("none") << false << false << QStringLiteral("DTR=off RTS=off");
    QTest::newRow("dtr") << true << false << QStringLiteral("DTR=on RTS=off");
    QTest::newRow("rts") << false << true << QStringLiteral("DTR=off RTS=on");
    QTest::newRow("both") << true << true << QStringLiteral("DTR=on RTS=on");
}

void SerialStationConfigTest::summaryIncludesControlLineStates()
{
    QFETCH(bool, dtrEnabled);
    QFETCH(bool, rtsEnabled);
    QFETCH(QString, suffix);

    SerialPortConfig config;
    config.portName = QStringLiteral("COM_LINES");
    config.dtrEnabled = dtrEnabled;
    config.rtsEnabled = rtsEnabled;

    QVERIFY(config.summary().endsWith(suffix));
}

void SerialStationConfigTest::reconnectRequiresPositiveInterval()
{
    SerialStationConfig config;
    config.autoReconnect = true;
    config.reconnectIntervalMs = 0;

    QVERIFY(!config.isReconnectEnabled());

    config.reconnectIntervalMs = -1;

    QVERIFY(!config.isReconnectEnabled());
}

void SerialStationConfigTest::reconnectDisabledWhenFlagIsFalse()
{
    SerialStationConfig config;
    config.autoReconnect = false;
    config.reconnectIntervalMs = 1500;

    QVERIFY(!config.isReconnectEnabled());
}

void SerialStationConfigTest::reconnectAllowsCustomInterval()
{
    SerialStationConfig config;
    config.autoReconnect = true;
    config.reconnectIntervalMs = 2500;

    QVERIFY(config.isReconnectEnabled());
}

QTEST_MAIN(SerialStationConfigTest)
#include "test_serial_station_config.moc"
