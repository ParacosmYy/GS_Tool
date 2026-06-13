#include <QtTest/QtTest>

#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QTemporaryDir>

#include "apps/serial_station/services/SerialMeasurementExportService.h"
#include "apps/serial_station/services/SerialMeasurementService.h"

using namespace serial_station;

class SerialMeasurementExportServiceTest : public QObject {
    Q_OBJECT

private slots:
    void formatSnapshotUsesMeasurementCsv();
    void exportWritesCsvFileAndReportsBytes();
    void exportWithBomWritesUtf8Bom();
    void exportRejectsEmptyPath();
    void exportRejectsEmptySnapshot();
    void exportRejectsMissingParentDirectory();
};

namespace {

SerialProtocolEvent measurementEvent(std::initializer_list<double> values)
{
    QVariantList variants;
    for (const double value : values) {
        variants.append(value);
    }

    SerialProtocolEvent event;
    event.type = QStringLiteral("measurement");
    event.protocolName = QStringLiteral("just_float");
    event.payload.insert(QStringLiteral("values"), variants);
    return event;
}

SerialMeasurementSnapshot sampleSnapshot()
{
    SerialMeasurementService service;
    service.appendEvent(measurementEvent({1.0, 2.0}));
    service.appendEvent(measurementEvent({3.5, 4.5}));
    return service.snapshot();
}

SerialMeasurementExportRequest requestFor(const QString& path)
{
    SerialMeasurementExportRequest request;
    request.filePath = path;
    return request;
}

QByteArray readAll(const QString& path)
{
    QFile file(path);
    const bool opened = file.open(QIODevice::ReadOnly);
    Q_ASSERT(opened);
    return file.readAll();
}

} // namespace

void SerialMeasurementExportServiceTest::formatSnapshotUsesMeasurementCsv()
{
    const SerialMeasurementExportService service;

    const QString csv = service.formatSnapshot(sampleSnapshot());

    QVERIFY(csv.startsWith(QStringLiteral("frame,ch1,ch2\n")));
    QVERIFY(csv.contains(QStringLiteral("1,1,2")));
    QVERIFY(csv.contains(QStringLiteral("2,3.5,4.5")));
}

void SerialMeasurementExportServiceTest::exportWritesCsvFileAndReportsBytes()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("measurement.csv"));
    const SerialMeasurementExportService service;

    const SerialMeasurementExportResult result =
        service.exportSnapshot(sampleSnapshot(), requestFor(path));

    QVERIFY(result.ok);
    QCOMPARE(result.filePath, QFileInfo(path).absoluteFilePath());
    QCOMPARE(result.format, QStringLiteral("csv"));
    QCOMPARE(result.bytesWritten, readAll(path).size());
    QVERIFY(readAll(path).contains("frame,ch1,ch2"));
}

void SerialMeasurementExportServiceTest::exportWithBomWritesUtf8Bom()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    SerialMeasurementExportRequest request = requestFor(dir.filePath(QStringLiteral("bom.csv")));
    request.writeUtf8Bom = true;
    const SerialMeasurementExportService service;

    const SerialMeasurementExportResult result =
        service.exportSnapshot(sampleSnapshot(), request);

    QVERIFY(result.ok);
    const QByteArray payload = readAll(request.filePath);
    QVERIFY(payload.startsWith(QByteArray::fromHex("EFBBBF")));
}

void SerialMeasurementExportServiceTest::exportRejectsEmptyPath()
{
    const SerialMeasurementExportService service;

    const SerialMeasurementExportResult result =
        service.exportSnapshot(sampleSnapshot(), requestFor(QString()));

    QVERIFY(!result.ok);
    QVERIFY(result.errorMessage.contains(QStringLiteral("路径")));
}

void SerialMeasurementExportServiceTest::exportRejectsEmptySnapshot()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const SerialMeasurementExportService service;

    const SerialMeasurementExportResult result =
        service.exportSnapshot(SerialMeasurementSnapshot(),
                               requestFor(dir.filePath(QStringLiteral("empty.csv"))));

    QVERIFY(!result.ok);
    QVERIFY(result.errorMessage.contains(QStringLiteral("测量")));
}

void SerialMeasurementExportServiceTest::exportRejectsMissingParentDirectory()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("missing/measurement.csv"));
    const SerialMeasurementExportService service;

    const SerialMeasurementExportResult result =
        service.exportSnapshot(sampleSnapshot(), requestFor(path));

    QVERIFY(!result.ok);
    QVERIFY(result.errorMessage.contains(QStringLiteral("目录")));
}

QTEST_MAIN(SerialMeasurementExportServiceTest)
#include "test_serial_measurement_export_service.moc"
