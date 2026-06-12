#include <QtTest/QtTest>

#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QRegularExpression>
#include <QtCore/QTemporaryDir>

#include "apps/serial_station/SerialStationController.h"

using serial_station::SerialExportFormat;
using serial_station::SerialExportRequest;
using serial_station::SerialExportResult;
using serial_station::SerialLogDirection;
using serial_station::SerialLogRecord;
using serial_station::SerialStationController;

class SerialStationExportFlowTest : public QObject {
    Q_OBJECT

private slots:
    void suggestedFileNamesUseRequestedSuffix();
    void jsonExportWritesExistingControllerRecords();
    void successExportEmitsSystemLog();
    void emptyLogExportFailsAndEmitsErrorLog();
    void csvExportThroughControllerWritesHeaderAndRows();
    void missingParentFailureDoesNotCreateTargetFile();
    void plainTextExportKeepsResultInspectable();
    void successLogIsNotIncludedInExportedSnapshot();
    void failedExportAppendsStructuredErrorRecord();
    void exportedJsonLinesRemainParseableObjects();
    void repeatedExportsAppendSeparateSystemRecords();
    void exportedPayloadBytesMatchReportedCount();
};

namespace {

void seedMixedLogs(SerialStationController& controller)
{
    controller.sendCommand(QStringLiteral("PING"), QStringLiteral("ascii"));
    controller.handleBytesReceived(QByteArray("OK\n"));
    controller.handleBytesReceived(QByteArray("PART"));
}

QByteArray readAll(const QString& path)
{
    QFile file(path);
    const bool opened = file.open(QIODevice::ReadOnly);
    Q_ASSERT(opened);
    return file.readAll();
}

SerialExportRequest requestFor(const QString& path, SerialExportFormat format)
{
    SerialExportRequest request;
    request.filePath = path;
    request.format = format;
    return request;
}

} // namespace

void SerialStationExportFlowTest::suggestedFileNamesUseRequestedSuffix()
{
    SerialStationController controller;

    const QString jsonName = controller.suggestedExportFileName(SerialExportFormat::JsonLines);
    const QString textName = controller.suggestedExportFileName(SerialExportFormat::PlainText);
    const QString csvName = controller.suggestedExportFileName(SerialExportFormat::Csv);

    QVERIFY(jsonName.startsWith(QStringLiteral("serial-log-")));
    QVERIFY(textName.startsWith(QStringLiteral("serial-log-")));
    QVERIFY(csvName.startsWith(QStringLiteral("serial-log-")));
    QVERIFY(jsonName.endsWith(QStringLiteral(".jsonl")));
    QVERIFY(textName.endsWith(QStringLiteral(".txt")));
    QVERIFY(csvName.endsWith(QStringLiteral(".csv")));

    const QRegularExpression stampPattern(QStringLiteral("^serial-log-\\d{8}-\\d{6}\\."));
    QVERIFY(stampPattern.match(jsonName).hasMatch());
    QVERIFY(stampPattern.match(textName).hasMatch());
    QVERIFY(stampPattern.match(csvName).hasMatch());
}

void SerialStationExportFlowTest::jsonExportWritesExistingControllerRecords()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    SerialStationController controller;
    seedMixedLogs(controller);

    const QString path = dir.filePath(QStringLiteral("session.jsonl"));
    const SerialExportResult result =
        controller.exportLogRecords(requestFor(path, SerialExportFormat::JsonLines));

    QVERIFY(result.ok);
    QCOMPARE(result.filePath, QFileInfo(path).absoluteFilePath());
    QCOMPARE(result.format, QStringLiteral("json_lines"));
    QVERIFY(result.bytesWritten > 0);

    const QString payload = QString::fromUtf8(readAll(path));
    const QStringList lines = payload.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    QCOMPARE(lines.count(), 3);
    QVERIFY(payload.contains(QStringLiteral("\"direction\":\"error\"")));
    QVERIFY(payload.contains(QStringLiteral("\"direction\":\"rx\"")));
    QVERIFY(payload.contains(QStringLiteral("\"direction\":\"system\"")));
}

void SerialStationExportFlowTest::successExportEmitsSystemLog()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    SerialStationController controller;
    seedMixedLogs(controller);
    QSignalSpy systemSpy(&controller, &SerialStationController::serialSystemLogged);

    const SerialExportResult result = controller.exportLogRecords(
        requestFor(dir.filePath(QStringLiteral("session.jsonl")), SerialExportFormat::JsonLines));

    QVERIFY(result.ok);
    QCOMPARE(systemSpy.count(), 1);
    QVERIFY(systemSpy.takeFirst().at(0).toString().contains(QStringLiteral("日志已导出")));

    const QVector<SerialLogRecord> records = controller.logRecords();
    QCOMPARE(records.last().direction, SerialLogDirection::System);
    QCOMPARE(records.last().fields.value(QStringLiteral("format")).toString(),
             QStringLiteral("json_lines"));
    QCOMPARE(records.last().fields.value(QStringLiteral("bytesWritten")).toLongLong(),
             result.bytesWritten);
}

void SerialStationExportFlowTest::emptyLogExportFailsAndEmitsErrorLog()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    SerialStationController controller;
    QSignalSpy systemSpy(&controller, &SerialStationController::serialSystemLogged);

    const SerialExportResult result = controller.exportLogRecords(
        requestFor(dir.filePath(QStringLiteral("empty.jsonl")), SerialExportFormat::JsonLines));

    QVERIFY(!result.ok);
    QVERIFY(result.errorMessage.contains(QStringLiteral("没有可导出的日志记录")));
    QCOMPARE(systemSpy.count(), 1);
    QVERIFY(systemSpy.takeFirst().at(0).toString().contains(QStringLiteral("日志导出失败")));
    QVERIFY(!QFileInfo::exists(result.filePath));

    const QVector<SerialLogRecord> records = controller.logRecords();
    QCOMPARE(records.count(), 1);
    QCOMPARE(records.first().direction, SerialLogDirection::Error);
    QCOMPARE(records.first().fields.value(QStringLiteral("format")).toString(),
             QStringLiteral("json_lines"));
}

void SerialStationExportFlowTest::csvExportThroughControllerWritesHeaderAndRows()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    SerialStationController controller;
    seedMixedLogs(controller);

    const QString path = dir.filePath(QStringLiteral("session.csv"));
    const SerialExportResult result =
        controller.exportLogRecords(requestFor(path, SerialExportFormat::Csv));

    QVERIFY(result.ok);
    QCOMPARE(result.format, QStringLiteral("csv"));

    const QString payload = QString::fromUtf8(readAll(path));
    const QStringList lines = payload.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    QCOMPARE(lines.first(), QStringLiteral("timestamp,direction,source,text,payloadHex"));
    QCOMPARE(lines.count(), 4);
    QVERIFY(payload.contains(QStringLiteral(",error,controller,")));
    QVERIFY(payload.contains(QStringLiteral(",rx,protocol,OK,4F 4B 0A")));
}

void SerialStationExportFlowTest::missingParentFailureDoesNotCreateTargetFile()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    SerialStationController controller;
    seedMixedLogs(controller);

    const QString path = dir.filePath(QStringLiteral("missing/session.txt"));
    const SerialExportResult result =
        controller.exportLogRecords(requestFor(path, SerialExportFormat::PlainText));

    QVERIFY(!result.ok);
    QVERIFY(result.errorMessage.contains(QStringLiteral("导出目录不存在")));
    QCOMPARE(result.bytesWritten, 0);
    QVERIFY(!QFileInfo::exists(path));
    QCOMPARE(controller.logRecords().last().direction, SerialLogDirection::Error);
}

void SerialStationExportFlowTest::plainTextExportKeepsResultInspectable()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    SerialStationController controller;
    seedMixedLogs(controller);

    const QString path = dir.filePath(QStringLiteral("session.txt"));
    const SerialExportResult result =
        controller.exportLogRecords(requestFor(path, SerialExportFormat::PlainText));

    QVERIFY(result.ok);
    QCOMPARE(result.format, QStringLiteral("plain_text"));
    QVERIFY(result.errorMessage.isEmpty());
    QCOMPARE(result.filePath, QFileInfo(path).absoluteFilePath());

    const QString payload = QString::fromUtf8(readAll(path));
    QVERIFY(payload.contains(QStringLiteral("ERROR")));
    QVERIFY(payload.contains(QStringLiteral("RX")));
    QVERIFY(payload.contains(QStringLiteral("SYSTEM")));
}

void SerialStationExportFlowTest::successLogIsNotIncludedInExportedSnapshot()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    SerialStationController controller;
    seedMixedLogs(controller);

    const QString path = dir.filePath(QStringLiteral("snapshot.jsonl"));
    const SerialExportResult result =
        controller.exportLogRecords(requestFor(path, SerialExportFormat::JsonLines));

    QVERIFY(result.ok);
    const QString payload = QString::fromUtf8(readAll(path));
    QVERIFY(!payload.contains(QStringLiteral("日志已导出")));
    QCOMPARE(controller.logRecords().count(), 4);
    QVERIFY(controller.logRecords().last().text.contains(QStringLiteral("日志已导出")));
}

void SerialStationExportFlowTest::failedExportAppendsStructuredErrorRecord()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    SerialStationController controller;
    seedMixedLogs(controller);

    SerialExportRequest request = requestFor(QString(), SerialExportFormat::Csv);
    const SerialExportResult result = controller.exportLogRecords(request);

    QVERIFY(!result.ok);
    QCOMPARE(result.format, QStringLiteral("csv"));

    const SerialLogRecord record = controller.logRecords().last();
    QCOMPARE(record.direction, SerialLogDirection::Error);
    QCOMPARE(record.fields.value(QStringLiteral("format")).toString(), QStringLiteral("csv"));
    QVERIFY(record.fields.value(QStringLiteral("message")).toString()
                .contains(QStringLiteral("导出路径为空")));
}

void SerialStationExportFlowTest::exportedJsonLinesRemainParseableObjects()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    SerialStationController controller;
    seedMixedLogs(controller);

    const QString path = dir.filePath(QStringLiteral("parseable.jsonl"));
    const SerialExportResult result =
        controller.exportLogRecords(requestFor(path, SerialExportFormat::JsonLines));
    QVERIFY(result.ok);

    const QStringList lines =
        QString::fromUtf8(readAll(path)).split(QLatin1Char('\n'), Qt::SkipEmptyParts);

    for (const QString& line : lines) {
        const QJsonDocument document = QJsonDocument::fromJson(line.toUtf8());
        QVERIFY(document.isObject());
        QVERIFY(document.object().contains(QStringLiteral("timestamp")));
        QVERIFY(document.object().contains(QStringLiteral("direction")));
        QVERIFY(document.object().contains(QStringLiteral("text")));
    }
}

void SerialStationExportFlowTest::repeatedExportsAppendSeparateSystemRecords()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    SerialStationController controller;
    seedMixedLogs(controller);
    QSignalSpy systemSpy(&controller, &SerialStationController::serialSystemLogged);

    const SerialExportResult first = controller.exportLogRecords(
        requestFor(dir.filePath(QStringLiteral("first.jsonl")), SerialExportFormat::JsonLines));
    const SerialExportResult second = controller.exportLogRecords(
        requestFor(dir.filePath(QStringLiteral("second.txt")), SerialExportFormat::PlainText));

    QVERIFY(first.ok);
    QVERIFY(second.ok);
    QCOMPARE(systemSpy.count(), 2);
    QCOMPARE(controller.logRecords().count(), 5);
    QCOMPARE(controller.logRecords().at(3).direction, SerialLogDirection::System);
    QCOMPARE(controller.logRecords().at(4).direction, SerialLogDirection::System);
}

void SerialStationExportFlowTest::exportedPayloadBytesMatchReportedCount()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    SerialStationController controller;
    seedMixedLogs(controller);

    const QString path = dir.filePath(QStringLiteral("bytes.csv"));
    const SerialExportResult result =
        controller.exportLogRecords(requestFor(path, SerialExportFormat::Csv));

    QVERIFY(result.ok);
    QCOMPARE(result.bytesWritten, readAll(path).size());
    QVERIFY(result.bytesWritten > 20);
}

QTEST_MAIN(SerialStationExportFlowTest)
#include "test_serial_station_export_flow.moc"
