#include <QtTest/QtTest>

#include <memory>

#include "apps/serial_station/SerialStationConstants.h"
#include "apps/serial_station/core/SerialDispatcher.h"
#include "apps/serial_station/protocols/ascii_text/AsciiTextProtocol.h"

using serial_station::AsciiTextProtocol;
using serial_station::ISerialProtocol;
using serial_station::SerialDispatcher;
using serial_station::SerialProtocolEvent;

namespace {

class CountingProtocol final : public ISerialProtocol {
public:
    QString name() const override
    {
        return QStringLiteral("counting");
    }

    QByteArray buildCommand(const QString& command, const QVariantMap& params) const override
    {
        Q_UNUSED(params)
        return command.toUtf8();
    }

    QVector<SerialProtocolEvent> feed(const QByteArray& data) override
    {
        ++feedCount;
        lastBytes = data;
        if (data == QByteArray("event")) {
            SerialProtocolEvent event;
            event.type = serial_station::serialStationConstants::kAsciiFrameType;
            event.protocolName = name();
            event.payload.insert(QStringLiteral("text"), QStringLiteral("event"));
            event.raw = data;
            return {event};
        }
        return {};
    }

    void reset() override
    {
        ++resetCount;
        lastBytes.clear();
    }

    int feedCount = 0;
    int resetCount = 0;
    QByteArray lastBytes;
};

} // namespace

class SerialDispatcherTest : public QObject {
    Q_OBJECT

private slots:
    void defaultStateHasNoProtocol();
    void missingProtocolRecordsDroppedBytes();
    void emptyBytesDoNotCallProtocol();
    void setProtocolResetsProtocolState();
    void asciiCompleteLineProducesFrameEvent();
    void asciiSplitLineBuffersUntilNewline();
    void resetClearsAsciiPartialBuffer();
    void multipleAsciiLinesProduceMultipleEvents();
    void oversizedAsciiBufferClearsAndStartsFresh();
    void protocolReplacementDropsOldBuffer();
    void bufferedStatusKeepsProtocolName();
    void emptyFeedKeepsActiveProtocolName();
    void eventStatusReportsEventCount();
    void resetKeepsProtocolAndClearsPartialBuffer();
    void protocolReceivesOriginalBytes();
};

void SerialDispatcherTest::defaultStateHasNoProtocol()
{
    SerialDispatcher dispatcher;

    QVERIFY(!dispatcher.hasProtocol());
    QVERIFY(dispatcher.protocolName().isEmpty());
    QCOMPARE(dispatcher.lastFeedSummary().status, SerialDispatcher::FeedStatus::EmptyInput);
    QCOMPARE(dispatcher.lastFeedSummary().inputBytes, 0);
    QCOMPARE(dispatcher.lastFeedSummary().eventCount, 0);
}

void SerialDispatcherTest::missingProtocolRecordsDroppedBytes()
{
    SerialDispatcher dispatcher;

    const QVector<SerialProtocolEvent> events = dispatcher.feed(QByteArray("OK\n"));

    QVERIFY(events.isEmpty());
    QCOMPARE(dispatcher.lastFeedSummary().status, SerialDispatcher::FeedStatus::MissingProtocol);
    QCOMPARE(dispatcher.lastFeedSummary().inputBytes, 3);
    QCOMPARE(dispatcher.lastFeedSummary().eventCount, 0);
    QVERIFY(dispatcher.lastFeedSummary().protocolName.isEmpty());
}

void SerialDispatcherTest::emptyBytesDoNotCallProtocol()
{
    SerialDispatcher dispatcher;
    auto protocol = std::make_unique<CountingProtocol>();
    CountingProtocol* rawProtocol = protocol.get();
    dispatcher.setProtocol(std::move(protocol));

    const QVector<SerialProtocolEvent> events = dispatcher.feed(QByteArray());

    QVERIFY(events.isEmpty());
    QCOMPARE(rawProtocol->feedCount, 0);
    QCOMPARE(dispatcher.lastFeedSummary().status, SerialDispatcher::FeedStatus::EmptyInput);
    QCOMPARE(dispatcher.lastFeedSummary().inputBytes, 0);
}

void SerialDispatcherTest::setProtocolResetsProtocolState()
{
    SerialDispatcher dispatcher;
    auto protocol = std::make_unique<CountingProtocol>();
    CountingProtocol* rawProtocol = protocol.get();

    dispatcher.setProtocol(std::move(protocol));

    QVERIFY(dispatcher.hasProtocol());
    QCOMPARE(dispatcher.protocolName(), QStringLiteral("counting"));
    QCOMPARE(rawProtocol->resetCount, 1);
    QCOMPARE(dispatcher.lastFeedSummary().status, SerialDispatcher::FeedStatus::EmptyInput);
}

void SerialDispatcherTest::asciiCompleteLineProducesFrameEvent()
{
    SerialDispatcher dispatcher;
    dispatcher.setProtocol(std::make_unique<AsciiTextProtocol>());

    const QVector<SerialProtocolEvent> events = dispatcher.feed(QByteArray("OK\n"));

    QCOMPARE(events.count(), 1);
    QCOMPARE(events.first().type, serial_station::serialStationConstants::kAsciiFrameType);
    QCOMPARE(events.first().payload.value(QStringLiteral("text")).toString(), QStringLiteral("OK"));
    QCOMPARE(dispatcher.lastFeedSummary().status, SerialDispatcher::FeedStatus::EventsReady);
    QCOMPARE(dispatcher.lastFeedSummary().eventCount, 1);
}

void SerialDispatcherTest::asciiSplitLineBuffersUntilNewline()
{
    SerialDispatcher dispatcher;
    dispatcher.setProtocol(std::make_unique<AsciiTextProtocol>());

    QVector<SerialProtocolEvent> events = dispatcher.feed(QByteArray("O"));

    QVERIFY(events.isEmpty());
    QCOMPARE(dispatcher.lastFeedSummary().status, SerialDispatcher::FeedStatus::Buffered);
    QCOMPARE(dispatcher.lastFeedSummary().inputBytes, 1);

    events = dispatcher.feed(QByteArray("K\n"));

    QCOMPARE(events.count(), 1);
    QCOMPARE(events.first().payload.value(QStringLiteral("text")).toString(), QStringLiteral("OK"));
    QCOMPARE(dispatcher.lastFeedSummary().status, SerialDispatcher::FeedStatus::EventsReady);
}

void SerialDispatcherTest::resetClearsAsciiPartialBuffer()
{
    SerialDispatcher dispatcher;
    dispatcher.setProtocol(std::make_unique<AsciiTextProtocol>());

    dispatcher.feed(QByteArray("STALE"));
    dispatcher.reset();
    const QVector<SerialProtocolEvent> events = dispatcher.feed(QByteArray("\n"));

    QCOMPARE(events.count(), 1);
    QCOMPARE(events.first().raw, QByteArray("\n"));
    QCOMPARE(events.first().payload.value(QStringLiteral("text")).toString(), QString());
}

void SerialDispatcherTest::multipleAsciiLinesProduceMultipleEvents()
{
    SerialDispatcher dispatcher;
    dispatcher.setProtocol(std::make_unique<AsciiTextProtocol>());

    const QVector<SerialProtocolEvent> events = dispatcher.feed(QByteArray("A\nB\nC\n"));

    QCOMPARE(events.count(), 3);
    QCOMPARE(events.at(0).payload.value(QStringLiteral("text")).toString(), QStringLiteral("A"));
    QCOMPARE(events.at(1).payload.value(QStringLiteral("text")).toString(), QStringLiteral("B"));
    QCOMPARE(events.at(2).payload.value(QStringLiteral("text")).toString(), QStringLiteral("C"));
    QCOMPARE(dispatcher.lastFeedSummary().eventCount, 3);
}

void SerialDispatcherTest::oversizedAsciiBufferClearsAndStartsFresh()
{
    SerialDispatcher dispatcher;
    dispatcher.setProtocol(std::make_unique<AsciiTextProtocol>());
    const QByteArray oversized(9000, 'X');

    QVector<SerialProtocolEvent> events = dispatcher.feed(oversized);
    QVERIFY(events.isEmpty());
    QCOMPARE(dispatcher.lastFeedSummary().status, SerialDispatcher::FeedStatus::Buffered);

    events = dispatcher.feed(QByteArray("OK\n"));

    QCOMPARE(events.count(), 1);
    QCOMPARE(events.first().payload.value(QStringLiteral("text")).toString(), QStringLiteral("OK"));
}

void SerialDispatcherTest::protocolReplacementDropsOldBuffer()
{
    SerialDispatcher dispatcher;
    dispatcher.setProtocol(std::make_unique<AsciiTextProtocol>());
    dispatcher.feed(QByteArray("OLD"));

    dispatcher.setProtocol(std::make_unique<AsciiTextProtocol>());
    const QVector<SerialProtocolEvent> events = dispatcher.feed(QByteArray("NEW\n"));

    QCOMPARE(events.count(), 1);
    QCOMPARE(events.first().payload.value(QStringLiteral("text")).toString(), QStringLiteral("NEW"));
}

void SerialDispatcherTest::bufferedStatusKeepsProtocolName()
{
    SerialDispatcher dispatcher;
    dispatcher.setProtocol(std::make_unique<AsciiTextProtocol>());

    dispatcher.feed(QByteArray("PARTIAL"));

    QCOMPARE(dispatcher.lastFeedSummary().status, SerialDispatcher::FeedStatus::Buffered);
    QCOMPARE(dispatcher.lastFeedSummary().inputBytes, 7);
    QCOMPARE(dispatcher.lastFeedSummary().eventCount, 0);
    QCOMPARE(dispatcher.lastFeedSummary().protocolName, QStringLiteral("ascii_text"));
}

void SerialDispatcherTest::emptyFeedKeepsActiveProtocolName()
{
    SerialDispatcher dispatcher;
    dispatcher.setProtocol(std::make_unique<AsciiTextProtocol>());

    const QVector<SerialProtocolEvent> events = dispatcher.feed(QByteArray());

    QVERIFY(events.isEmpty());
    QCOMPARE(dispatcher.lastFeedSummary().status, SerialDispatcher::FeedStatus::EmptyInput);
    QCOMPARE(dispatcher.lastFeedSummary().inputBytes, 0);
    QCOMPARE(dispatcher.lastFeedSummary().eventCount, 0);
    QCOMPARE(dispatcher.lastFeedSummary().protocolName, QStringLiteral("ascii_text"));
}

void SerialDispatcherTest::eventStatusReportsEventCount()
{
    SerialDispatcher dispatcher;
    dispatcher.setProtocol(std::make_unique<CountingProtocol>());

    const QVector<SerialProtocolEvent> events = dispatcher.feed(QByteArray("event"));

    QCOMPARE(events.count(), 1);
    QCOMPARE(dispatcher.lastFeedSummary().status, SerialDispatcher::FeedStatus::EventsReady);
    QCOMPARE(dispatcher.lastFeedSummary().inputBytes, 5);
    QCOMPARE(dispatcher.lastFeedSummary().eventCount, 1);
    QCOMPARE(dispatcher.lastFeedSummary().protocolName, QStringLiteral("counting"));
}

void SerialDispatcherTest::resetKeepsProtocolAndClearsPartialBuffer()
{
    SerialDispatcher dispatcher;
    dispatcher.setProtocol(std::make_unique<AsciiTextProtocol>());
    dispatcher.feed(QByteArray("PARTIAL"));

    dispatcher.reset();
    const QVector<SerialProtocolEvent> events = dispatcher.feed(QByteArray("READY\n"));

    QVERIFY(dispatcher.hasProtocol());
    QCOMPARE(dispatcher.protocolName(), QStringLiteral("ascii_text"));
    QCOMPARE(events.count(), 1);
    QCOMPARE(events.first().payload.value(QStringLiteral("text")).toString(), QStringLiteral("READY"));
    QCOMPARE(dispatcher.lastFeedSummary().status, SerialDispatcher::FeedStatus::EventsReady);
}

void SerialDispatcherTest::protocolReceivesOriginalBytes()
{
    SerialDispatcher dispatcher;
    auto protocol = std::make_unique<CountingProtocol>();
    CountingProtocol* rawProtocol = protocol.get();
    dispatcher.setProtocol(std::move(protocol));

    const QByteArray bytes("\x01\x02RAW", 5);
    const QVector<SerialProtocolEvent> events = dispatcher.feed(bytes);

    QVERIFY(events.isEmpty());
    QCOMPARE(rawProtocol->feedCount, 1);
    QCOMPARE(rawProtocol->lastBytes, bytes);
    QCOMPARE(dispatcher.lastFeedSummary().status, SerialDispatcher::FeedStatus::Buffered);
    QCOMPARE(dispatcher.lastFeedSummary().inputBytes, bytes.size());
}

QTEST_MAIN(SerialDispatcherTest)
#include "test_serial_dispatcher.moc"
