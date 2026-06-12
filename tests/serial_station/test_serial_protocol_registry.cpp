#include <QtTest/QtTest>

#include "apps/serial_station/protocols/SerialProtocolRegistry.h"
#include "apps/serial_station/protocols/ascii_text/AsciiTextProtocol.h"

using namespace serial_station;

class SerialProtocolRegistryTest : public QObject {
    Q_OBJECT

private slots:
    void registerAndCreateProtocol();
    void rejectDuplicateProtocolName();
    void defaultProtocolMustExist();
    void builtInProtocolsIncludeModbusRtu();
};

void SerialProtocolRegistryTest::registerAndCreateProtocol()
{
    SerialProtocolRegistry registry;

    QVERIFY(registry.registerProtocol(QStringLiteral("ascii_text"), [] {
        return std::make_unique<AsciiTextProtocol>();
    }));

    QVERIFY(registry.contains(QStringLiteral("ASCII_TEXT")));
    QCOMPARE(registry.protocolNames(), QStringList{QStringLiteral("ascii_text")});
    QVERIFY(registry.create(QStringLiteral("ascii_text")) != nullptr);
}

void SerialProtocolRegistryTest::rejectDuplicateProtocolName()
{
    SerialProtocolRegistry registry;

    QVERIFY(registry.registerProtocol(QStringLiteral("ascii_text"), [] {
        return std::make_unique<AsciiTextProtocol>();
    }));
    QVERIFY(!registry.registerProtocol(QStringLiteral(" ASCII_TEXT "), [] {
        return std::make_unique<AsciiTextProtocol>();
    }));
}

void SerialProtocolRegistryTest::defaultProtocolMustExist()
{
    SerialProtocolRegistry registry;

    QVERIFY(!registry.setDefaultProtocol(QStringLiteral("missing")));
    QVERIFY(registry.registerProtocol(QStringLiteral("ascii_text"), [] {
        return std::make_unique<AsciiTextProtocol>();
    }));
    QVERIFY(registry.setDefaultProtocol(QStringLiteral("ASCII_TEXT")));
    QCOMPARE(registry.defaultProtocol(), QStringLiteral("ascii_text"));
    QVERIFY(registry.createDefault() != nullptr);
}

void SerialProtocolRegistryTest::builtInProtocolsIncludeModbusRtu()
{
    SerialProtocolRegistry registry;

    registry.registerBuiltInProtocols();

    QVERIFY(registry.contains(QStringLiteral("ascii_text")));
    QVERIFY(registry.contains(QStringLiteral("modbus_rtu")));
    QCOMPARE(registry.defaultProtocol(), QStringLiteral("ascii_text"));
    QVERIFY(registry.create(QStringLiteral("modbus_rtu")) != nullptr);
}

QTEST_MAIN(SerialProtocolRegistryTest)
#include "test_serial_protocol_registry.moc"
