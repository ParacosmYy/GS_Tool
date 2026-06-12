#include <QtTest/QtTest>

#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>

#include "apps/serial_station/services/SerialLogService.h"
#include "apps/serial_station/services/SerialReplayService.h"

using serial_station::SerialLogDirection;
using serial_station::SerialLogRecord;
using serial_station::SerialLogService;
using serial_station::SerialReplayEvent;
using serial_station::SerialReplayOptions;
using serial_station::SerialReplayPlan;
using serial_station::SerialReplayService;

class SerialReplayServiceTest : public QObject {
    Q_OBJECT

private slots:
    void defaultPlanKeepsTxAndRxOnly();
    void includeFlagsCanKeepAllDirections();
    void delaysAreComputedFromTimestamps();
    void speedMultiplierScalesDelays();
    void maxDelayClampsLongGaps();
    void negativeMaxDelayDisablesClamp();
    void invalidSpeedFallsBackToNormalSpeed();
    void negativeTimestampDeltaClampsToZero();
    void emptyRecordsFail();
    void allFilteredRecordsFail();
    void jsonLinesRoundTripFromLogService();
    void invalidJsonLineFailsWithLineNumber();
    void missingDirectionFails();
    void missingTextFails();
    void invalidPayloadHexFails();
    void compactPayloadHexParses();
    void eventSummaryContainsReplayFacts();
    void planSummaryReportsDurationAndSkippedRecords();
};

namespace {

QDateTime atMs(int ms)
{
    return QDateTime::fromString(
        QStringLiteral("2026-06-12T08:00:00.%1").arg(ms, 3, 10, QLatin1Char('0')),
        Qt::ISODateWithMs);
}

SerialLogRecord record(SerialLogDirection direction,
                       int ms,
                       const QString& text,
                       const QByteArray& payload = QByteArray())
{
    SerialLogRecord item;
    item.timestamp = atMs(ms);
    item.direction = direction;
    item.source = QStringLiteral("test");
    item.text = text;
    item.payload = payload;
    return item;
}

QVector<SerialLogRecord> mixedRecords()
{
    return {
        record(SerialLogDirection::System, 0, QStringLiteral("session start")),
        record(SerialLogDirection::Tx, 100, QStringLiteral("PING"), QByteArray("PING")),
        record(SerialLogDirection::Rx, 350, QStringLiteral("OK"), QByteArray("OK\n")),
        record(SerialLogDirection::Error, 900, QStringLiteral("timeout"))
    };
}

QString jsonLine(const QJsonObject& object)
{
    const QJsonDocument document(object);
    return QString::fromUtf8(document.toJson(QJsonDocument::Compact));
}

} // namespace

void SerialReplayServiceTest::defaultPlanKeepsTxAndRxOnly()
{
    const SerialReplayService service;
    const SerialReplayPlan plan = service.buildPlan(mixedRecords());

    QVERIFY(plan.ok);
    QCOMPARE(plan.events.count(), 2);
    QCOMPARE(plan.skippedRecords, 2);
    QCOMPARE(plan.events.at(0).direction, SerialLogDirection::Tx);
    QCOMPARE(plan.events.at(1).direction, SerialLogDirection::Rx);
    QCOMPARE(plan.events.at(0).text, QStringLiteral("PING"));
    QCOMPARE(plan.events.at(1).payload, QByteArray("OK\n"));
}

void SerialReplayServiceTest::includeFlagsCanKeepAllDirections()
{
    SerialReplayOptions options;
    options.includeSystem = true;
    options.includeError = true;

    const SerialReplayService service;
    const SerialReplayPlan plan = service.buildPlan(mixedRecords(), options);

    QVERIFY(plan.ok);
    QCOMPARE(plan.events.count(), 4);
    QCOMPARE(plan.skippedRecords, 0);
    QCOMPARE(plan.events.at(0).direction, SerialLogDirection::System);
    QCOMPARE(plan.events.at(3).direction, SerialLogDirection::Error);
}

void SerialReplayServiceTest::delaysAreComputedFromTimestamps()
{
    const SerialReplayService service;
    const SerialReplayPlan plan = service.buildPlan(mixedRecords());

    QVERIFY(plan.ok);
    QCOMPARE(plan.events.at(0).delayMs, 0);
    QCOMPARE(plan.events.at(1).delayMs, 250);
    QCOMPARE(plan.totalDurationMs, 250);
}

void SerialReplayServiceTest::speedMultiplierScalesDelays()
{
    SerialReplayOptions options;
    options.speedMultiplier = 2.0;

    const SerialReplayService service;
    const SerialReplayPlan plan = service.buildPlan(mixedRecords(), options);

    QVERIFY(plan.ok);
    QCOMPARE(plan.events.at(1).delayMs, 125);
    QCOMPARE(plan.totalDurationMs, 125);
}

void SerialReplayServiceTest::maxDelayClampsLongGaps()
{
    SerialReplayOptions options;
    options.maxDelayMs = 100;

    const SerialReplayService service;
    const SerialReplayPlan plan = service.buildPlan(mixedRecords(), options);

    QVERIFY(plan.ok);
    QCOMPARE(plan.events.at(1).delayMs, 100);
    QCOMPARE(plan.totalDurationMs, 100);
}

void SerialReplayServiceTest::negativeMaxDelayDisablesClamp()
{
    SerialReplayOptions options;
    options.maxDelayMs = -1;

    const SerialReplayService service;
    const SerialReplayPlan plan = service.buildPlan(mixedRecords(), options);

    QVERIFY(plan.ok);
    QCOMPARE(plan.events.at(1).delayMs, 250);
}

void SerialReplayServiceTest::invalidSpeedFallsBackToNormalSpeed()
{
    SerialReplayOptions options;
    options.speedMultiplier = 0.0;

    const SerialReplayService service;
    const SerialReplayPlan plan = service.buildPlan(mixedRecords(), options);

    QVERIFY(plan.ok);
    QCOMPARE(plan.events.at(1).delayMs, 250);
}

void SerialReplayServiceTest::negativeTimestampDeltaClampsToZero()
{
    QVector<SerialLogRecord> records;
    records.append(record(SerialLogDirection::Tx, 900, QStringLiteral("LATE")));
    records.append(record(SerialLogDirection::Rx, 100, QStringLiteral("EARLY")));

    const SerialReplayService service;
    const SerialReplayPlan plan = service.buildPlan(records);

    QVERIFY(plan.ok);
    QCOMPARE(plan.events.count(), 2);
    QCOMPARE(plan.events.at(1).delayMs, 0);
    QCOMPARE(plan.totalDurationMs, 0);
}

void SerialReplayServiceTest::emptyRecordsFail()
{
    const SerialReplayService service;
    const SerialReplayPlan plan = service.buildPlan({});

    QVERIFY(!plan.ok);
    QVERIFY(plan.errorMessage.contains(QStringLiteral("没有可回放")));
    QVERIFY(plan.events.isEmpty());
    QCOMPARE(plan.totalDurationMs, 0);
}

void SerialReplayServiceTest::allFilteredRecordsFail()
{
    SerialReplayOptions options;
    options.includeTx = false;
    options.includeRx = false;
    options.includeSystem = false;
    options.includeError = false;

    const SerialReplayService service;
    const SerialReplayPlan plan = service.buildPlan(mixedRecords(), options);

    QVERIFY(!plan.ok);
    QVERIFY(plan.errorMessage.contains(QStringLiteral("过滤后")));
    QVERIFY(plan.events.isEmpty());
}

void SerialReplayServiceTest::jsonLinesRoundTripFromLogService()
{
    SerialLogService logService;
    QVERIFY(logService.append(record(SerialLogDirection::Tx,
                                     100,
                                     QStringLiteral("PING"),
                                     QByteArray("PING"))));
    QVERIFY(logService.append(record(SerialLogDirection::Rx,
                                     350,
                                     QStringLiteral("OK"),
                                     QByteArray("OK\n"))));

    const SerialReplayService service;
    const SerialReplayPlan plan = service.buildPlanFromJsonLines(logService.toJsonLines());

    QVERIFY(plan.ok);
    QCOMPARE(plan.events.count(), 2);
    QCOMPARE(plan.events.at(0).payload, QByteArray("PING"));
    QCOMPARE(plan.events.at(1).payload, QByteArray("OK\n"));
    QCOMPARE(plan.events.at(1).delayMs, 250);
}

void SerialReplayServiceTest::invalidJsonLineFailsWithLineNumber()
{
    const SerialReplayService service;
    const SerialReplayPlan plan =
        service.buildPlanFromJsonLines(QStringLiteral("{\"direction\":\"tx\",\"text\":\"OK\"}\n{"));

    QVERIFY(!plan.ok);
    QVERIFY(plan.errorMessage.contains(QStringLiteral("第 2 行")));
    QVERIFY(plan.errorMessage.contains(QStringLiteral("JSON 无效")));
}

void SerialReplayServiceTest::missingDirectionFails()
{
    const SerialReplayService service;
    const SerialReplayPlan plan = service.buildPlanFromJsonLines(
        jsonLine({{QStringLiteral("text"), QStringLiteral("PING")}}));

    QVERIFY(!plan.ok);
    QVERIFY(plan.errorMessage.contains(QStringLiteral("direction 无效")));
}

void SerialReplayServiceTest::missingTextFails()
{
    const SerialReplayService service;
    const SerialReplayPlan plan = service.buildPlanFromJsonLines(
        jsonLine({{QStringLiteral("direction"), QStringLiteral("tx")},
                  {QStringLiteral("text"), QStringLiteral("   ")}}));

    QVERIFY(!plan.ok);
    QVERIFY(plan.errorMessage.contains(QStringLiteral("缺少 text")));
}

void SerialReplayServiceTest::invalidPayloadHexFails()
{
    const SerialReplayService service;
    const SerialReplayPlan plan = service.buildPlanFromJsonLines(
        jsonLine({{QStringLiteral("direction"), QStringLiteral("tx")},
                  {QStringLiteral("text"), QStringLiteral("PING")},
                  {QStringLiteral("payloadHex"), QStringLiteral("GG")}}));

    QVERIFY(!plan.ok);
    QVERIFY(plan.errorMessage.contains(QStringLiteral("payloadHex 包含非法字符")));
}

void SerialReplayServiceTest::compactPayloadHexParses()
{
    const SerialReplayService service;
    const SerialReplayPlan plan = service.buildPlanFromJsonLines(
        jsonLine({{QStringLiteral("direction"), QStringLiteral("rx")},
                  {QStringLiteral("text"), QStringLiteral("OK")},
                  {QStringLiteral("payloadHex"), QStringLiteral("4F4B0A")}}));

    QVERIFY(plan.ok);
    QCOMPARE(plan.events.count(), 1);
    QCOMPARE(plan.events.first().payload, QByteArray("OK\n"));
}

void SerialReplayServiceTest::eventSummaryContainsReplayFacts()
{
    SerialReplayEvent event;
    event.direction = SerialLogDirection::Rx;
    event.timestamp = atMs(123);
    event.source = QStringLiteral("protocol");
    event.text = QStringLiteral("OK");
    event.payload = QByteArray("OK\n");
    event.delayMs = 25;

    const SerialReplayService service;
    const QString summary = service.eventSummary(event);

    QVERIFY(summary.contains(QStringLiteral("+25ms")));
    QVERIFY(summary.contains(QStringLiteral("RX")));
    QVERIFY(summary.contains(QStringLiteral("protocol")));
    QVERIFY(summary.contains(QStringLiteral("OK")));
    QVERIFY(summary.contains(QStringLiteral("hex=4F 4B 0A")));
}

void SerialReplayServiceTest::planSummaryReportsDurationAndSkippedRecords()
{
    const SerialReplayService service;
    const SerialReplayPlan plan = service.buildPlan(mixedRecords());
    const QString summary = service.planSummary(plan);

    QVERIFY(plan.ok);
    QVERIFY(summary.contains(QStringLiteral("2 events")));
    QVERIFY(summary.contains(QStringLiteral("250 ms")));
    QVERIFY(summary.contains(QStringLiteral("skipped 2")));

    const SerialReplayPlan failed = service.buildPlan({});
    QVERIFY(service.planSummary(failed).contains(QStringLiteral("Replay failed")));
}

QTEST_MAIN(SerialReplayServiceTest)
#include "test_serial_replay_service.moc"
