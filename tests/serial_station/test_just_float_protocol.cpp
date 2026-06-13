#include <QtTest/QtTest>

#include <QtCore/QDataStream>
#include <QtCore/QIODevice>

#include "apps/serial_station/protocols/just_float/JustFloatProtocol.h"

using namespace serial_station;

class JustFloatProtocolTest : public QObject {
    Q_OBJECT

private slots:
    void nameReturnsStableProtocolId();
    void buildCommandReturnsTrimmedAsciiBytes();
    void buildCommandCanAppendNewline();
    void parsesSingleFloatFrame();
    void parsesMultipleFloatsInOneFrame();
    void parsesSplitFrameAcrossFeeds();
    void parsesMultipleFramesInOneFeed();
    void keepsPartialFrameUntilTailArrives();
    void invalidPayloadLengthEmitsErrorAndResynchronizes();
    void resetClearsBufferedPartialFrame();
    void resetRestartsFrameIndex();
    void emptyBytesProduceNoEvents();
    void handlesNegativeAndFractionalValues();
    void rawFrameContainsPayloadAndTail();
    void measurementPayloadContainsChannelCount();
};

namespace {

QByteArray justFloatTail()
{
    return QByteArray::fromHex("0000807F");
}

QByteArray floatBytes(float value)
{
    QByteArray bytes;
    QDataStream stream(&bytes, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    stream << value;
    return bytes;
}

QByteArray justFloatFrame(std::initializer_list<float> values)
{
    QByteArray frame;
    for (const float value : values) {
        frame.append(floatBytes(value));
    }
    frame.append(justFloatTail());
    return frame;
}

QVariantList eventValues(const SerialProtocolEvent& event)
{
    return event.payload.value(QStringLiteral("values")).toList();
}

void compareFloatVariant(const QVariant& actual, double expected)
{
    QVERIFY(qAbs(actual.toDouble() - expected) < 0.0001);
}

} // namespace

void JustFloatProtocolTest::nameReturnsStableProtocolId()
{
    JustFloatProtocol protocol;

    QCOMPARE(protocol.name(), QStringLiteral("just_float"));
}

void JustFloatProtocolTest::buildCommandReturnsTrimmedAsciiBytes()
{
    JustFloatProtocol protocol;

    QCOMPARE(protocol.buildCommand(QStringLiteral("  start_stream  "), QVariantMap()),
             QByteArray("start_stream"));
}

void JustFloatProtocolTest::buildCommandCanAppendNewline()
{
    JustFloatProtocol protocol;
    QVariantMap params;
    params.insert(QStringLiteral("appendNewline"), true);

    QCOMPARE(protocol.buildCommand(QStringLiteral("sample"), params), QByteArray("sample\n"));
}

void JustFloatProtocolTest::parsesSingleFloatFrame()
{
    JustFloatProtocol protocol;

    const QVector<SerialProtocolEvent> events = protocol.feed(justFloatFrame({1.25F}));

    QCOMPARE(events.size(), 1);
    QCOMPARE(events.first().type, QStringLiteral("measurement"));
    QCOMPARE(events.first().protocolName, QStringLiteral("just_float"));
    const QVariantList values = eventValues(events.first());
    QCOMPARE(values.size(), 1);
    compareFloatVariant(values.first(), 1.25);
}

void JustFloatProtocolTest::parsesMultipleFloatsInOneFrame()
{
    JustFloatProtocol protocol;

    const QVector<SerialProtocolEvent> events = protocol.feed(justFloatFrame({1.0F, 2.5F, 3.75F}));

    QCOMPARE(events.size(), 1);
    const QVariantList values = eventValues(events.first());
    QCOMPARE(values.size(), 3);
    compareFloatVariant(values.at(0), 1.0);
    compareFloatVariant(values.at(1), 2.5);
    compareFloatVariant(values.at(2), 3.75);
}

void JustFloatProtocolTest::parsesSplitFrameAcrossFeeds()
{
    JustFloatProtocol protocol;
    const QByteArray frame = justFloatFrame({42.0F});

    QVERIFY(protocol.feed(frame.left(2)).isEmpty());
    const QVector<SerialProtocolEvent> events = protocol.feed(frame.mid(2));

    QCOMPARE(events.size(), 1);
    compareFloatVariant(eventValues(events.first()).first(), 42.0);
}

void JustFloatProtocolTest::parsesMultipleFramesInOneFeed()
{
    JustFloatProtocol protocol;
    const QByteArray bytes = justFloatFrame({1.0F}) + justFloatFrame({2.0F});

    const QVector<SerialProtocolEvent> events = protocol.feed(bytes);

    QCOMPARE(events.size(), 2);
    compareFloatVariant(eventValues(events.at(0)).first(), 1.0);
    compareFloatVariant(eventValues(events.at(1)).first(), 2.0);
    QCOMPARE(events.at(0).payload.value(QStringLiteral("frameIndex")).toInt(), 1);
    QCOMPARE(events.at(1).payload.value(QStringLiteral("frameIndex")).toInt(), 2);
}

void JustFloatProtocolTest::keepsPartialFrameUntilTailArrives()
{
    JustFloatProtocol protocol;
    const QByteArray payload = floatBytes(7.0F);

    QVERIFY(protocol.feed(payload).isEmpty());
    const QVector<SerialProtocolEvent> events = protocol.feed(justFloatTail());

    QCOMPARE(events.size(), 1);
    compareFloatVariant(eventValues(events.first()).first(), 7.0);
}

void JustFloatProtocolTest::invalidPayloadLengthEmitsErrorAndResynchronizes()
{
    JustFloatProtocol protocol;
    QByteArray bytes("abc", 3);
    bytes.append(justFloatTail());
    bytes.append(justFloatFrame({9.0F}));

    const QVector<SerialProtocolEvent> events = protocol.feed(bytes);

    QCOMPARE(events.size(), 2);
    QCOMPARE(events.at(0).type, QStringLiteral("error"));
    QCOMPARE(events.at(0).protocolName, QStringLiteral("just_float"));
    QCOMPARE(events.at(0).payload.value(QStringLiteral("payloadSize")).toInt(), 3);
    QCOMPARE(events.at(1).type, QStringLiteral("measurement"));
    compareFloatVariant(eventValues(events.at(1)).first(), 9.0);
}

void JustFloatProtocolTest::resetClearsBufferedPartialFrame()
{
    JustFloatProtocol protocol;
    protocol.feed(floatBytes(5.0F));
    protocol.reset();

    const QVector<SerialProtocolEvent> events = protocol.feed(justFloatTail());

    QCOMPARE(events.size(), 1);
    QCOMPARE(events.first().type, QStringLiteral("error"));
}

void JustFloatProtocolTest::resetRestartsFrameIndex()
{
    JustFloatProtocol protocol;
    QCOMPARE(protocol.feed(justFloatFrame({1.0F})).first().payload.value(QStringLiteral("frameIndex")).toInt(), 1);

    protocol.reset();
    const QVector<SerialProtocolEvent> events = protocol.feed(justFloatFrame({2.0F}));

    QCOMPARE(events.first().payload.value(QStringLiteral("frameIndex")).toInt(), 1);
}

void JustFloatProtocolTest::emptyBytesProduceNoEvents()
{
    JustFloatProtocol protocol;

    QVERIFY(protocol.feed(QByteArray()).isEmpty());
}

void JustFloatProtocolTest::handlesNegativeAndFractionalValues()
{
    JustFloatProtocol protocol;

    const QVector<SerialProtocolEvent> events = protocol.feed(justFloatFrame({-12.5F, 0.125F}));

    QCOMPARE(events.size(), 1);
    const QVariantList values = eventValues(events.first());
    compareFloatVariant(values.at(0), -12.5);
    compareFloatVariant(values.at(1), 0.125);
}

void JustFloatProtocolTest::rawFrameContainsPayloadAndTail()
{
    JustFloatProtocol protocol;
    const QByteArray frame = justFloatFrame({3.0F});

    const QVector<SerialProtocolEvent> events = protocol.feed(frame);

    QCOMPARE(events.first().raw, frame);
}

void JustFloatProtocolTest::measurementPayloadContainsChannelCount()
{
    JustFloatProtocol protocol;

    const QVector<SerialProtocolEvent> events = protocol.feed(justFloatFrame({1.0F, 2.0F, 3.0F, 4.0F}));

    QCOMPARE(events.first().payload.value(QStringLiteral("format")).toString(), QStringLiteral("just_float"));
    QCOMPARE(events.first().payload.value(QStringLiteral("channelCount")).toInt(), 4);
}

QTEST_MAIN(JustFloatProtocolTest)
#include "test_just_float_protocol.moc"
