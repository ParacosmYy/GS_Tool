#include <QtTest/QtTest>

#include "apps/serial_station/SerialStationConstants.h"
#include "apps/serial_station/protocols/modbus_rtu/ModbusRtuProtocol.h"

using namespace serial_station;

class ModbusRtuProtocolTest : public QObject {
    Q_OBJECT

private slots:
    void buildReadHoldingRegistersRequest();
    void buildWriteSingleRegisterRequest();
    void buildReadCoilsAndWriteSingleCoilRequests();
    void rejectInvalidParameters();
    void parseReadResponseAfterPartialInput();
    void parseStickyResponses();
    void parseExceptionResponse();
    void reportCrcMismatch();
    void resetDropsBufferedPartialFrame();
};

void ModbusRtuProtocolTest::buildReadHoldingRegistersRequest()
{
    ModbusRtuProtocol protocol;
    QVariantMap params;
    params.insert(QStringLiteral("slaveId"), 1);
    params.insert(QStringLiteral("startAddress"), 0);
    params.insert(QStringLiteral("quantity"), 2);

    const QByteArray frame = protocol.buildCommand(QStringLiteral("read_holding_registers"), params);

    QCOMPARE(frame.toHex(' ').toUpper(), QByteArray("01 03 00 00 00 02 C4 0B"));
    QVERIFY(protocol.lastError().isEmpty());
}

void ModbusRtuProtocolTest::buildWriteSingleRegisterRequest()
{
    ModbusRtuProtocol protocol;
    QVariantMap params;
    params.insert(QStringLiteral("slaveId"), 1);
    params.insert(QStringLiteral("address"), 1);
    params.insert(QStringLiteral("value"), 0x1234);

    const QByteArray frame = protocol.buildCommand(QStringLiteral("write_single_register"), params);

    QCOMPARE(frame.toHex(' ').toUpper(), QByteArray("01 06 00 01 12 34 D5 7D"));
}

void ModbusRtuProtocolTest::buildReadCoilsAndWriteSingleCoilRequests()
{
    ModbusRtuProtocol protocol;
    QVariantMap readParams;
    readParams.insert(QStringLiteral("slaveId"), 1);
    readParams.insert(QStringLiteral("startAddress"), 0);
    readParams.insert(QStringLiteral("quantity"), 8);

    const QByteArray readFrame = protocol.buildCommand(QStringLiteral("read_coils"), readParams);
    QCOMPARE(readFrame.left(6).toHex(' ').toUpper(), QByteArray("01 01 00 00 00 08"));

    QVariantMap writeParams;
    writeParams.insert(QStringLiteral("slaveId"), 1);
    writeParams.insert(QStringLiteral("address"), 2);
    writeParams.insert(QStringLiteral("value"), true);

    const QByteArray writeFrame = protocol.buildCommand(QStringLiteral("write_single_coil"), writeParams);
    QCOMPARE(writeFrame.left(6).toHex(' ').toUpper(), QByteArray("01 05 00 02 FF 00"));
}

void ModbusRtuProtocolTest::rejectInvalidParameters()
{
    ModbusRtuProtocol protocol;
    QVariantMap params;
    params.insert(QStringLiteral("slaveId"), 0);
    params.insert(QStringLiteral("startAddress"), 0);
    params.insert(QStringLiteral("quantity"), 2);

    QVERIFY(protocol.buildCommand(QStringLiteral("read_holding_registers"), params).isEmpty());
    QVERIFY(protocol.lastError().contains(QStringLiteral("slaveId")));

    params.insert(QStringLiteral("slaveId"), 1);
    params.insert(QStringLiteral("quantity"), 126);
    QVERIFY(protocol.buildCommand(QStringLiteral("read_holding_registers"), params).isEmpty());
    QVERIFY(protocol.lastError().contains(QStringLiteral("quantity")));

    QVERIFY(protocol.buildCommand(QStringLiteral("missing"), params).isEmpty());
    QVERIFY(protocol.lastError().contains(QStringLiteral("unsupported")));
}

void ModbusRtuProtocolTest::parseReadResponseAfterPartialInput()
{
    ModbusRtuProtocol protocol;
    const QByteArray firstPart = QByteArray::fromHex("010304");
    const QByteArray secondPart = QByteArray::fromHex("000A0014DA3E");

    QVERIFY(protocol.feed(firstPart).isEmpty());
    const QVector<SerialProtocolEvent> events = protocol.feed(secondPart);

    QCOMPARE(events.size(), 1);
    QCOMPARE(events.first().type, serialStationConstants::kModbusFrameType);
    QCOMPARE(events.first().protocolName, QStringLiteral("modbus_rtu"));
    QCOMPARE(events.first().payload.value(QStringLiteral("slaveId")).toInt(), 1);
    QCOMPARE(events.first().payload.value(QStringLiteral("functionCode")).toInt(), 3);
    QCOMPARE(events.first().payload.value(QStringLiteral("byteCount")).toInt(), 4);
    QCOMPARE(events.first().payload.value(QStringLiteral("data")).toByteArray().toHex().toUpper(), QByteArray("000A0014"));
}

void ModbusRtuProtocolTest::parseStickyResponses()
{
    ModbusRtuProtocol protocol;
    const QByteArray responseA = QByteArray::fromHex("010304000A0014DA3E");
    const QByteArray responseB = QByteArray::fromHex("010600020001E9CA");

    const QVector<SerialProtocolEvent> events = protocol.feed(responseA + responseB);

    QCOMPARE(events.size(), 2);
    QCOMPARE(events.at(0).payload.value(QStringLiteral("functionCode")).toInt(), 3);
    QCOMPARE(events.at(1).payload.value(QStringLiteral("functionCode")).toInt(), 6);
    QCOMPARE(events.at(1).payload.value(QStringLiteral("address")).toInt(), 2);
    QCOMPARE(events.at(1).payload.value(QStringLiteral("value")).toInt(), 1);
}

void ModbusRtuProtocolTest::parseExceptionResponse()
{
    ModbusRtuProtocol protocol;
    const QByteArray exceptionFrame = QByteArray::fromHex("018302C0F1");

    const QVector<SerialProtocolEvent> events = protocol.feed(exceptionFrame);

    QCOMPARE(events.size(), 1);
    QCOMPARE(events.first().type, serialStationConstants::kModbusFrameType);
    QVERIFY(events.first().payload.value(QStringLiteral("exception")).toBool());
    QCOMPARE(events.first().payload.value(QStringLiteral("functionCode")).toInt(), 0x83);
    QCOMPARE(events.first().payload.value(QStringLiteral("exceptionCode")).toInt(), 2);
}

void ModbusRtuProtocolTest::reportCrcMismatch()
{
    ModbusRtuProtocol protocol;
    const QByteArray badFrame = QByteArray::fromHex("010304000A00140000");

    const QVector<SerialProtocolEvent> events = protocol.feed(badFrame);

    QCOMPARE(events.size(), 1);
    QCOMPARE(events.first().type, serialStationConstants::kModbusErrorType);
    QVERIFY(events.first().payload.value(QStringLiteral("message")).toString().contains(QStringLiteral("CRC")));
    QVERIFY(protocol.lastError().contains(QStringLiteral("CRC")));
}

void ModbusRtuProtocolTest::resetDropsBufferedPartialFrame()
{
    ModbusRtuProtocol protocol;

    QVERIFY(protocol.feed(QByteArray::fromHex("010304")).isEmpty());
    protocol.reset();

    const QVector<SerialProtocolEvent> events = protocol.feed(QByteArray::fromHex("000A"));
    QVERIFY(events.isEmpty());
}

QTEST_MAIN(ModbusRtuProtocolTest)
#include "test_modbus_rtu_protocol.moc"
