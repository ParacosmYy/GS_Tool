#include <QtTest/QtTest>

#include "apps/serial_station/SerialStationConstants.h"
#include "apps/serial_station/protocols/custom_md/CustomMdProtocol.h"

using namespace serial_station;

class CustomMdProtocolTest : public QObject {
    Q_OBJECT

private slots:
    void buildDefaultFrameFromHexPayload();
    void buildFrameFromByteArrayPayload();
    void buildFrameWithCustomHeaderFooterAndNoChecksum();
    void rejectInvalidInputs();
    void parseFrameAfterPartialInput();
    void parseStickyFrames();
    void resyncAfterNoisePrefix();
    void reportChecksumMismatch();
    void resetDropsBufferedPartialFrame();
};

void CustomMdProtocolTest::buildDefaultFrameFromHexPayload()
{
    CustomMdProtocol protocol;
    QVariantMap params;
    params.insert(QStringLiteral("payload"), QStringLiteral("01 02"));

    const QByteArray frame = protocol.buildCommand(QStringLiteral("0x10"), params);

    QCOMPARE(frame.toHex(' ').toUpper(), QByteArray("A5 5A 03 10 01 02 16 0D 0A"));
    QVERIFY(protocol.lastError().isEmpty());
}

void CustomMdProtocolTest::buildFrameFromByteArrayPayload()
{
    CustomMdProtocol protocol;
    QVariantMap params;
    params.insert(QStringLiteral("payload"), QByteArray::fromHex("0102"));

    const QByteArray frame = protocol.buildCommand(QStringLiteral("10"), params);

    QCOMPARE(frame.toHex(' ').toUpper(), QByteArray("A5 5A 03 10 01 02 16 0D 0A"));
}

void CustomMdProtocolTest::buildFrameWithCustomHeaderFooterAndNoChecksum()
{
    CustomMdProtocol protocol;
    QVariantMap params;
    params.insert(QStringLiteral("header"), QStringLiteral("55 AA"));
    params.insert(QStringLiteral("footer"), QByteArray::fromHex("EE"));
    params.insert(QStringLiteral("checksum"), QStringLiteral("none"));
    params.insert(QStringLiteral("payload"), QStringLiteral("01 02"));

    const QByteArray frame = protocol.buildCommand(QStringLiteral("0x10"), params);

    QCOMPARE(frame.toHex(' ').toUpper(), QByteArray("55 AA 03 10 01 02 EE"));
}

void CustomMdProtocolTest::rejectInvalidInputs()
{
    CustomMdProtocol protocol;
    QVariantMap params;
    params.insert(QStringLiteral("payload"), QStringLiteral("0A 0"));

    QVERIFY(protocol.buildCommand(QStringLiteral("0x10"), params).isEmpty());
    QVERIFY(protocol.lastError().contains(QStringLiteral("payload")));

    params.insert(QStringLiteral("payload"), QByteArray(241, '\x01'));
    QVERIFY(protocol.buildCommand(QStringLiteral("0x10"), params).isEmpty());
    QVERIFY(protocol.lastError().contains(QStringLiteral("large")));

    params.insert(QStringLiteral("payload"), QByteArray());
    QVERIFY(protocol.buildCommand(QStringLiteral("0x100"), params).isEmpty());
    QVERIFY(protocol.lastError().contains(QStringLiteral("command")));
}

void CustomMdProtocolTest::parseFrameAfterPartialInput()
{
    CustomMdProtocol protocol;
    const QByteArray firstPart = QByteArray::fromHex("A55A0310");
    const QByteArray secondPart = QByteArray::fromHex("0102160D0A");

    QVERIFY(protocol.feed(firstPart).isEmpty());
    const QVector<SerialProtocolEvent> events = protocol.feed(secondPart);

    QCOMPARE(events.size(), 1);
    QCOMPARE(events.first().type, serialStationConstants::kCustomMdFrameType);
    QCOMPARE(events.first().protocolName, QStringLiteral("custom_md"));
    QCOMPARE(events.first().payload.value(QStringLiteral("length")).toInt(), 3);
    QCOMPARE(events.first().payload.value(QStringLiteral("commandCode")).toInt(), 0x10);
    QCOMPARE(events.first().payload.value(QStringLiteral("payload")).toByteArray().toHex().toUpper(), QByteArray("0102"));
    QCOMPARE(events.first().payload.value(QStringLiteral("checksum")).toInt(), 0x16);
}

void CustomMdProtocolTest::parseStickyFrames()
{
    CustomMdProtocol protocol;
    const QByteArray frameA = QByteArray::fromHex("A55A03100102160D0A");
    const QByteArray frameB = QByteArray::fromHex("A55A0120210D0A");

    const QVector<SerialProtocolEvent> events = protocol.feed(frameA + frameB);

    QCOMPARE(events.size(), 2);
    QCOMPARE(events.at(0).payload.value(QStringLiteral("commandCode")).toInt(), 0x10);
    QCOMPARE(events.at(1).payload.value(QStringLiteral("commandCode")).toInt(), 0x20);
    QVERIFY(events.at(1).payload.value(QStringLiteral("payload")).toByteArray().isEmpty());
}

void CustomMdProtocolTest::resyncAfterNoisePrefix()
{
    CustomMdProtocol protocol;
    const QByteArray noiseAndFrame = QByteArray::fromHex("000102A55A03100102160D0A");

    const QVector<SerialProtocolEvent> events = protocol.feed(noiseAndFrame);

    QCOMPARE(events.size(), 1);
    QCOMPARE(events.first().type, serialStationConstants::kCustomMdFrameType);
    QCOMPARE(events.first().payload.value(QStringLiteral("commandCode")).toInt(), 0x10);
}

void CustomMdProtocolTest::reportChecksumMismatch()
{
    CustomMdProtocol protocol;
    const QByteArray badFrame = QByteArray::fromHex("A55A03100102000D0A");

    const QVector<SerialProtocolEvent> events = protocol.feed(badFrame);

    QCOMPARE(events.size(), 1);
    QCOMPARE(events.first().type, serialStationConstants::kCustomMdErrorType);
    QVERIFY(events.first().payload.value(QStringLiteral("message")).toString().contains(QStringLiteral("checksum")));
    QVERIFY(protocol.lastError().contains(QStringLiteral("checksum")));
}

void CustomMdProtocolTest::resetDropsBufferedPartialFrame()
{
    CustomMdProtocol protocol;

    QVERIFY(protocol.feed(QByteArray::fromHex("A55A0310")).isEmpty());
    protocol.reset();

    QVERIFY(protocol.feed(QByteArray::fromHex("010216")).isEmpty());
}

QTEST_MAIN(CustomMdProtocolTest)
#include "test_custom_md_protocol.moc"
