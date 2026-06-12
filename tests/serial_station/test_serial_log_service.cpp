#include <QtTest/QtTest>

#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>

#include "apps/serial_station/services/SerialLogService.h"

using serial_station::SerialLogDirection;
using serial_station::SerialLogFilter;
using serial_station::SerialLogRecord;
using serial_station::SerialLogService;

class SerialLogServiceTest : public QObject {
    Q_OBJECT

private slots:
    void appendPreservesOrderAndDirections();
    void convenienceMethodsNormalizeRecords();
    void rejectsEmptyRecords();
    void maxRecordCountEvictsOldest();
    void setMaxRecordCountClampsAndTrimsImmediately();
    void filtersByDirectionSourceTextAndPayload();
    void filtersByTimeRange();
    void plainTextContainsStableFields();
    void jsonLinesContainsStableKeysAndEscapedValues();
    void clearRemovesAllRecords();
};

void SerialLogServiceTest::appendPreservesOrderAndDirections()
{
    SerialLogService service;

    SerialLogRecord rx;
    rx.timestamp = QDateTime::fromString(QStringLiteral("2026-06-12T08:00:00.001"),
                                         Qt::ISODateWithMs);
    rx.direction = SerialLogDirection::Rx;
    rx.source = QStringLiteral("COM1");
    rx.text = QStringLiteral("boot");

    SerialLogRecord tx;
    tx.timestamp = QDateTime::fromString(QStringLiteral("2026-06-12T08:00:01.002"),
                                         Qt::ISODateWithMs);
    tx.direction = SerialLogDirection::Tx;
    tx.source = QStringLiteral("COM1");
    tx.text = QStringLiteral("AT");

    QVERIFY(service.append(rx));
    QVERIFY(service.append(tx));

    const QVector<SerialLogRecord> records = service.records();
    QCOMPARE(records.count(), 2);
    QCOMPARE(records.at(0).direction, SerialLogDirection::Rx);
    QCOMPARE(records.at(0).text, QStringLiteral("boot"));
    QCOMPARE(records.at(1).direction, SerialLogDirection::Tx);
    QCOMPARE(records.at(1).text, QStringLiteral("AT"));
}

void SerialLogServiceTest::convenienceMethodsNormalizeRecords()
{
    SerialLogService service;

    QVERIFY(service.appendTx(QStringLiteral("  write register  "),
                             QByteArray::fromHex("010600010002"),
                             QStringLiteral("  command_panel  ")));
    QVERIFY(service.appendRx(QStringLiteral("  ok  "),
                             QByteArray::fromHex("010600010002"),
                             QStringLiteral("device")));
    QVERIFY(service.appendSystem(QStringLiteral("  connected  "), QString()));
    QVERIFY(service.appendError(QStringLiteral("  timeout  "), QStringLiteral("session")));

    const QVector<SerialLogRecord> records = service.records();
    QCOMPARE(records.count(), 4);
    QVERIFY(records.at(0).timestamp.isValid());
    QCOMPARE(records.at(0).direction, SerialLogDirection::Tx);
    QCOMPARE(records.at(0).source, QStringLiteral("command_panel"));
    QCOMPARE(records.at(0).text, QStringLiteral("write register"));
    QCOMPARE(records.at(1).direction, SerialLogDirection::Rx);
    QCOMPARE(records.at(2).direction, SerialLogDirection::System);
    QCOMPARE(records.at(2).source, QStringLiteral("serial_station"));
    QCOMPARE(records.at(3).direction, SerialLogDirection::Error);
}

void SerialLogServiceTest::rejectsEmptyRecords()
{
    SerialLogService service;

    SerialLogRecord blank;
    blank.direction = SerialLogDirection::System;
    blank.text = QStringLiteral(" \t\n ");
    blank.source = QStringLiteral("controller");

    QVERIFY(!service.append(blank));
    QVERIFY(!service.appendTx(QStringLiteral("   ")));
    QCOMPARE(service.count(), 0);

    SerialLogRecord payloadOnly;
    payloadOnly.direction = SerialLogDirection::Rx;
    payloadOnly.payload = QByteArray::fromHex("aa55");
    QVERIFY(service.append(payloadOnly));
    QCOMPARE(service.count(), 1);
}

void SerialLogServiceTest::maxRecordCountEvictsOldest()
{
    SerialLogService service(3);

    QVERIFY(service.appendSystem(QStringLiteral("one")));
    QVERIFY(service.appendSystem(QStringLiteral("two")));
    QVERIFY(service.appendSystem(QStringLiteral("three")));
    QVERIFY(service.appendSystem(QStringLiteral("four")));

    const QVector<SerialLogRecord> records = service.records();
    QCOMPARE(records.count(), 3);
    QCOMPARE(records.at(0).text, QStringLiteral("two"));
    QCOMPARE(records.at(1).text, QStringLiteral("three"));
    QCOMPARE(records.at(2).text, QStringLiteral("four"));
}

void SerialLogServiceTest::setMaxRecordCountClampsAndTrimsImmediately()
{
    SerialLogService service(4);

    QVERIFY(service.appendSystem(QStringLiteral("one")));
    QVERIFY(service.appendSystem(QStringLiteral("two")));
    QVERIFY(service.appendSystem(QStringLiteral("three")));
    QVERIFY(service.appendSystem(QStringLiteral("four")));

    service.setMaxRecords(2);

    QCOMPARE(service.maxRecords(), 2);
    QCOMPARE(service.count(), 2);
    QCOMPARE(service.records().at(0).text, QStringLiteral("three"));
    QCOMPARE(service.records().at(1).text, QStringLiteral("four"));

    service.setMaxRecords(0);
    QCOMPARE(service.maxRecords(), 1);
    QCOMPARE(service.count(), 1);
    QCOMPARE(service.records().first().text, QStringLiteral("four"));
}

void SerialLogServiceTest::filtersByDirectionSourceTextAndPayload()
{
    SerialLogService service;

    QVERIFY(service.appendTx(QStringLiteral("read holding register"),
                             QByteArray::fromHex("010300000002"),
                             QStringLiteral("command")));
    QVERIFY(service.appendRx(QStringLiteral("register response"),
                             QByteArray::fromHex("010304000A000B"),
                             QStringLiteral("device")));
    QVERIFY(service.appendError(QStringLiteral("crc mismatch"), QStringLiteral("parser")));

    SerialLogFilter txFilter;
    txFilter.directions = {SerialLogDirection::Tx};
    QCOMPARE(service.records(txFilter).count(), 1);
    QCOMPARE(service.records(txFilter).first().text, QStringLiteral("read holding register"));

    SerialLogFilter sourceFilter;
    sourceFilter.sourceContains = QStringLiteral("DEV");
    QCOMPARE(service.records(sourceFilter).count(), 1);
    QCOMPARE(service.records(sourceFilter).first().direction, SerialLogDirection::Rx);

    SerialLogFilter textFilter;
    textFilter.textContains = QStringLiteral("crc");
    QCOMPARE(service.records(textFilter).count(), 1);
    QCOMPARE(service.records(textFilter).first().direction, SerialLogDirection::Error);

    SerialLogFilter payloadFilter;
    payloadFilter.textContains = QStringLiteral("00 0A");
    QCOMPARE(service.records(payloadFilter).count(), 1);
    QCOMPARE(service.records(payloadFilter).first().source, QStringLiteral("device"));
}

void SerialLogServiceTest::filtersByTimeRange()
{
    SerialLogService service;

    SerialLogRecord early;
    early.timestamp = QDateTime::fromString(QStringLiteral("2026-06-12T08:00:00.000"),
                                            Qt::ISODateWithMs);
    early.direction = SerialLogDirection::System;
    early.text = QStringLiteral("early");

    SerialLogRecord middle;
    middle.timestamp = QDateTime::fromString(QStringLiteral("2026-06-12T08:00:05.000"),
                                             Qt::ISODateWithMs);
    middle.direction = SerialLogDirection::System;
    middle.text = QStringLiteral("middle");

    SerialLogRecord late;
    late.timestamp = QDateTime::fromString(QStringLiteral("2026-06-12T08:00:10.000"),
                                           Qt::ISODateWithMs);
    late.direction = SerialLogDirection::System;
    late.text = QStringLiteral("late");

    QVERIFY(service.append(early));
    QVERIFY(service.append(middle));
    QVERIFY(service.append(late));

    SerialLogFilter filter;
    filter.from = QDateTime::fromString(QStringLiteral("2026-06-12T08:00:03.000"),
                                        Qt::ISODateWithMs);
    filter.to = QDateTime::fromString(QStringLiteral("2026-06-12T08:00:07.000"),
                                      Qt::ISODateWithMs);

    const QVector<SerialLogRecord> records = service.records(filter);
    QCOMPARE(records.count(), 1);
    QCOMPARE(records.first().text, QStringLiteral("middle"));
}

void SerialLogServiceTest::plainTextContainsStableFields()
{
    SerialLogService service;

    SerialLogRecord record;
    record.timestamp = QDateTime::fromString(QStringLiteral("2026-06-12T08:00:00.123"),
                                             Qt::ISODateWithMs);
    record.direction = SerialLogDirection::Tx;
    record.source = QStringLiteral("command");
    record.text = QStringLiteral("write");
    record.payload = QByteArray::fromHex("AA5501");

    QVERIFY(service.append(record));

    const QString plainText = service.toPlainText();
    QVERIFY(plainText.contains(QStringLiteral("[2026-06-12T08:00:00.123]")));
    QVERIFY(plainText.contains(QStringLiteral("TX")));
    QVERIFY(plainText.contains(QStringLiteral("command")));
    QVERIFY(plainText.contains(QStringLiteral("write")));
    QVERIFY(plainText.contains(QStringLiteral("hex=AA 55 01")));
}

void SerialLogServiceTest::jsonLinesContainsStableKeysAndEscapedValues()
{
    SerialLogService service;

    QVariantMap fields;
    fields.insert(QStringLiteral("requestId"), 42);
    fields.insert(QStringLiteral("mode"), QStringLiteral("protocol"));

    SerialLogRecord record;
    record.timestamp = QDateTime::fromString(QStringLiteral("2026-06-12T08:00:00.123"),
                                             Qt::ISODateWithMs);
    record.direction = SerialLogDirection::Rx;
    record.source = QStringLiteral("parser");
    record.text = QStringLiteral("value \"quoted\"");
    record.payload = QByteArray::fromHex("010304000A");
    record.fields = fields;

    QVERIFY(service.append(record));

    const QString jsonLines = service.toJsonLines();
    QVERIFY(!jsonLines.contains(QLatin1Char('\n')));

    const QJsonDocument document = QJsonDocument::fromJson(jsonLines.toUtf8());
    QVERIFY(document.isObject());

    const QJsonObject object = document.object();
    QCOMPARE(object.value(QStringLiteral("timestamp")).toString(),
             QStringLiteral("2026-06-12T08:00:00.123"));
    QCOMPARE(object.value(QStringLiteral("direction")).toString(), QStringLiteral("rx"));
    QCOMPARE(object.value(QStringLiteral("source")).toString(), QStringLiteral("parser"));
    QCOMPARE(object.value(QStringLiteral("text")).toString(), QStringLiteral("value \"quoted\""));
    QCOMPARE(object.value(QStringLiteral("payloadHex")).toString(), QStringLiteral("01 03 04 00 0A"));
    QCOMPARE(object.value(QStringLiteral("fields")).toObject()
                 .value(QStringLiteral("mode")).toString(),
             QStringLiteral("protocol"));
}

void SerialLogServiceTest::clearRemovesAllRecords()
{
    SerialLogService service;

    QVERIFY(service.appendSystem(QStringLiteral("connected")));
    QVERIFY(service.appendError(QStringLiteral("timeout")));
    QCOMPARE(service.count(), 2);

    service.clear();

    QVERIFY(service.isEmpty());
    QCOMPARE(service.records().count(), 0);
    QVERIFY(service.toPlainText().isEmpty());
    QVERIFY(service.toJsonLines().isEmpty());
}

QTEST_MAIN(SerialLogServiceTest)
#include "test_serial_log_service.moc"
