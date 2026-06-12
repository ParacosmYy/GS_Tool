#include <QtTest/QtTest>

#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QTemporaryDir>

#include "apps/serial_station/services/SerialExportService.h"

using serial_station::SerialExportFormat;
using serial_station::SerialExportRequest;
using serial_station::SerialExportResult;
using serial_station::SerialExportService;
using serial_station::SerialLogDirection;
using serial_station::SerialLogRecord;

class SerialExportServiceTest : public QObject {
    Q_OBJECT

private slots:
    void formatNamesAndSuffixesAreStable();
    void plainTextFormattingContainsLogFacts();
    void jsonLinesFormattingParsesEachLine();
    void csvFormattingHasStableHeaderAndRows();
    void csvEscapesCommaQuoteAndNewline();
    void exportWritesUtf8FileAndReportsBytes();
    void exportWithBomWritesUtf8Bom();
    void exportRejectsEmptyPath();
    void exportRejectsEmptyRecords();
    void exportRejectsMissingParentDirectory();
    void exportRejectsUnsupportedFormat();
    void unsupportedFormatFormattingReturnsEmptyText();
    void jsonLinesFileKeepsOneObjectPerLine();
};

namespace {

QVector<SerialLogRecord> sampleRecords()
{
    QVector<SerialLogRecord> records;

    SerialLogRecord tx;
    tx.timestamp = QDateTime::fromString(QStringLiteral("2026-06-12T08:00:00.001"),
                                         Qt::ISODateWithMs);
    tx.direction = SerialLogDirection::Tx;
    tx.source = QStringLiteral("controller");
    tx.text = QStringLiteral("write register");
    tx.payload = QByteArray::fromHex("010600010002");
    records.append(tx);

    SerialLogRecord rx;
    rx.timestamp = QDateTime::fromString(QStringLiteral("2026-06-12T08:00:00.101"),
                                         Qt::ISODateWithMs);
    rx.direction = SerialLogDirection::Rx;
    rx.source = QStringLiteral("protocol");
    rx.text = QStringLiteral("register response");
    rx.payload = QByteArray::fromHex("010600010002");
    records.append(rx);

    SerialLogRecord error;
    error.timestamp = QDateTime::fromString(QStringLiteral("2026-06-12T08:00:01.000"),
                                            Qt::ISODateWithMs);
    error.direction = SerialLogDirection::Error;
    error.source = QStringLiteral("serial_manager");
    error.text = QStringLiteral("timeout");
    records.append(error);

    return records;
}

QByteArray readAll(const QString& path)
{
    QFile file(path);
    const bool opened = file.open(QIODevice::ReadOnly);
    Q_ASSERT(opened);
    return file.readAll();
}

} // namespace

void SerialExportServiceTest::formatNamesAndSuffixesAreStable()
{
    const SerialExportService service;

    QCOMPARE(service.formatName(SerialExportFormat::PlainText), QStringLiteral("plain_text"));
    QCOMPARE(service.formatName(SerialExportFormat::JsonLines), QStringLiteral("json_lines"));
    QCOMPARE(service.formatName(SerialExportFormat::Csv), QStringLiteral("csv"));
    QCOMPARE(service.defaultSuffix(SerialExportFormat::PlainText), QStringLiteral("txt"));
    QCOMPARE(service.defaultSuffix(SerialExportFormat::JsonLines), QStringLiteral("jsonl"));
    QCOMPARE(service.defaultSuffix(SerialExportFormat::Csv), QStringLiteral("csv"));
}

void SerialExportServiceTest::plainTextFormattingContainsLogFacts()
{
    const SerialExportService service;
    const QString text = service.formatRecords(sampleRecords(), SerialExportFormat::PlainText);

    QVERIFY(text.contains(QStringLiteral("TX")));
    QVERIFY(text.contains(QStringLiteral("RX")));
    QVERIFY(text.contains(QStringLiteral("ERROR")));
    QVERIFY(text.contains(QStringLiteral("controller")));
    QVERIFY(text.contains(QStringLiteral("write register")));
    QVERIFY(text.contains(QStringLiteral("hex=01 06 00 01 00 02")));
}

void SerialExportServiceTest::jsonLinesFormattingParsesEachLine()
{
    const SerialExportService service;
    const QString jsonLines = service.formatRecords(sampleRecords(), SerialExportFormat::JsonLines);
    const QStringList lines = jsonLines.split(QLatin1Char('\n'), Qt::SkipEmptyParts);

    QCOMPARE(lines.count(), 3);

    const QJsonDocument first = QJsonDocument::fromJson(lines.first().toUtf8());
    QVERIFY(first.isObject());
    QCOMPARE(first.object().value(QStringLiteral("direction")).toString(), QStringLiteral("tx"));
    QCOMPARE(first.object().value(QStringLiteral("source")).toString(),
             QStringLiteral("controller"));
    QCOMPARE(first.object().value(QStringLiteral("payloadHex")).toString(),
             QStringLiteral("01 06 00 01 00 02"));
}

void SerialExportServiceTest::csvFormattingHasStableHeaderAndRows()
{
    const SerialExportService service;
    const QString csv = service.formatRecords(sampleRecords(), SerialExportFormat::Csv);
    const QStringList lines = csv.split(QLatin1Char('\n'));

    QCOMPARE(lines.count(), 4);
    QCOMPARE(lines.at(0), QStringLiteral("timestamp,direction,source,text,payloadHex"));
    QCOMPARE(lines.at(1),
             QStringLiteral("2026-06-12T08:00:00.001,tx,controller,write register,01 06 00 01 00 02"));
    QCOMPARE(lines.at(2),
             QStringLiteral("2026-06-12T08:00:00.101,rx,protocol,register response,01 06 00 01 00 02"));
    QCOMPARE(lines.at(3),
             QStringLiteral("2026-06-12T08:00:01.000,error,serial_manager,timeout,"));
}

void SerialExportServiceTest::csvEscapesCommaQuoteAndNewline()
{
    SerialLogRecord record;
    record.timestamp = QDateTime::fromString(QStringLiteral("2026-06-12T08:00:02.000"),
                                             Qt::ISODateWithMs);
    record.direction = SerialLogDirection::System;
    record.source = QStringLiteral("controller");
    record.text = QStringLiteral("value,\"quoted\"\nnext");

    const SerialExportService service;
    const QString csv = service.formatRecords({record}, SerialExportFormat::Csv);

    QVERIFY(csv.contains(QStringLiteral("\"value,\"\"quoted\"\"")));
    QVERIFY(csv.contains(QStringLiteral("next\"")));
}

void SerialExportServiceTest::exportWritesUtf8FileAndReportsBytes()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString path = dir.filePath(QStringLiteral("serial-log.txt"));
    SerialExportRequest request;
    request.filePath = path;
    request.format = SerialExportFormat::PlainText;

    const SerialExportService service;
    const SerialExportResult result = service.exportRecords(sampleRecords(), request);

    QVERIFY(result.ok);
    QCOMPARE(result.filePath, QFileInfo(path).absoluteFilePath());
    QCOMPARE(result.format, QStringLiteral("plain_text"));
    QVERIFY(result.bytesWritten > 0);
    QVERIFY(result.errorMessage.isEmpty());

    const QByteArray payload = readAll(path);
    QCOMPARE(result.bytesWritten, payload.size());
    QVERIFY(QString::fromUtf8(payload).contains(QStringLiteral("write register")));
}

void SerialExportServiceTest::exportWithBomWritesUtf8Bom()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    SerialExportRequest request;
    request.filePath = dir.filePath(QStringLiteral("serial-log.csv"));
    request.format = SerialExportFormat::Csv;
    request.writeUtf8Bom = true;

    const SerialExportService service;
    const SerialExportResult result = service.exportRecords(sampleRecords(), request);

    QVERIFY(result.ok);
    const QByteArray payload = readAll(request.filePath);
    QVERIFY(payload.startsWith(QByteArray::fromHex("efbbbf")));
    QCOMPARE(result.bytesWritten, payload.size());
}

void SerialExportServiceTest::exportRejectsEmptyPath()
{
    SerialExportRequest request;
    request.filePath = QStringLiteral("   ");
    request.format = SerialExportFormat::PlainText;

    const SerialExportService service;
    const SerialExportResult result = service.exportRecords(sampleRecords(), request);

    QVERIFY(!result.ok);
    QCOMPARE(result.bytesWritten, 0);
    QVERIFY(result.errorMessage.contains(QStringLiteral("导出路径为空")));
}

void SerialExportServiceTest::exportRejectsEmptyRecords()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    SerialExportRequest request;
    request.filePath = dir.filePath(QStringLiteral("empty.txt"));
    request.format = SerialExportFormat::PlainText;

    const SerialExportService service;
    const SerialExportResult result = service.exportRecords({}, request);

    QVERIFY(!result.ok);
    QCOMPARE(result.bytesWritten, 0);
    QVERIFY(result.errorMessage.contains(QStringLiteral("没有可导出的日志记录")));
    QVERIFY(!QFileInfo::exists(request.filePath));
}

void SerialExportServiceTest::exportRejectsMissingParentDirectory()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    SerialExportRequest request;
    request.filePath = dir.filePath(QStringLiteral("missing/serial-log.txt"));
    request.format = SerialExportFormat::PlainText;

    const SerialExportService service;
    const SerialExportResult result = service.exportRecords(sampleRecords(), request);

    QVERIFY(!result.ok);
    QCOMPARE(result.bytesWritten, 0);
    QVERIFY(result.errorMessage.contains(QStringLiteral("导出目录不存在")));
    QVERIFY(!QFileInfo::exists(request.filePath));
}

void SerialExportServiceTest::exportRejectsUnsupportedFormat()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    SerialExportRequest request;
    request.filePath = dir.filePath(QStringLiteral("unsupported.bin"));
    request.format = static_cast<SerialExportFormat>(999);

    const SerialExportService service;
    const SerialExportResult result = service.exportRecords(sampleRecords(), request);

    QVERIFY(!result.ok);
    QCOMPARE(result.format, QStringLiteral("unsupported"));
    QCOMPARE(result.bytesWritten, 0);
    QVERIFY(result.errorMessage.contains(QStringLiteral("导出格式不受支持")));
    QVERIFY(!QFileInfo::exists(request.filePath));
}

void SerialExportServiceTest::unsupportedFormatFormattingReturnsEmptyText()
{
    const SerialExportFormat unsupported = static_cast<SerialExportFormat>(999);
    const SerialExportService service;

    QCOMPARE(service.formatName(unsupported), QStringLiteral("unsupported"));
    QVERIFY(service.defaultSuffix(unsupported).isEmpty());
    QVERIFY(service.formatRecords(sampleRecords(), unsupported).isEmpty());
}

void SerialExportServiceTest::jsonLinesFileKeepsOneObjectPerLine()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    SerialExportRequest request;
    request.filePath = dir.filePath(QStringLiteral("serial-log.jsonl"));
    request.format = SerialExportFormat::JsonLines;

    const SerialExportService service;
    const SerialExportResult result = service.exportRecords(sampleRecords(), request);

    QVERIFY(result.ok);

    const QString payload = QString::fromUtf8(readAll(request.filePath));
    const QStringList lines = payload.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    QCOMPARE(lines.count(), 3);

    for (const QString& line : lines) {
        const QJsonDocument document = QJsonDocument::fromJson(line.toUtf8());
        QVERIFY(document.isObject());
        QVERIFY(document.object().contains(QStringLiteral("timestamp")));
        QVERIFY(document.object().contains(QStringLiteral("direction")));
    }
}

QTEST_MAIN(SerialExportServiceTest)
#include "test_serial_export_service.moc"
