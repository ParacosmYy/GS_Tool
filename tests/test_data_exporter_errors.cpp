#include <QtTest/QtTest>

#include "utils/export/DataExporter.h"

#include <QSignalSpy>
#include <QTemporaryDir>

class DataExporterErrorsTest : public QObject {
    Q_OBJECT

private slots:
    void emptyLinesEmitExportError();
    void emptyPathEmitsExportError();
    void whitespacePathEmitsExportError();
    void streamedEmptyPathEmitsExportError();
    void streamedWhitespacePathEmitsExportError();
    void streamedEmptyInputEmitsExportError();
    void streamedProviderEmptyBatchDoesNotReportSuccess();
    void streamedNonPositiveBatchSizeFallsBackToDefaultBatch();
    void failedPlainExportDoesNotIncrementPlainExportCounter();
    void failedStreamedPlainExportDoesNotIncrementPlainExportCounter();
    void failedStreamedExportDoesNotIncrementTotalExports();
    void filteredOutLinesEmitExportError();
    void failedPlainExportDoesNotIncrementTotalExports();
    void filteredOutLinesDoNotIncrementTotalExports();
    void rangeExportEmptyPathsEmitExportError();
    void rangeExportWhitespacePathsEmitExportError();
    void rangeExportInvalidTimeRangeDoesNotIncrementTotalExports();
};

namespace {
TerminalLine makeLine(const QDateTime& timestamp)
{
    TerminalLine line;
    line.timestamp = timestamp;
    line.direction = DataDirection::Rx;
    line.data = QByteArray("abc");
    return line;
}
} // namespace

void DataExporterErrorsTest::emptyLinesEmitExportError()
{
    DataExporter exporter;
    QSignalSpy spy(&exporter, &DataExporter::exportError);

    const bool ok = exporter.exportToFile(QStringLiteral("capture.txt"),
                                          DataExporter::Plain,
                                          {});

    QVERIFY(!ok);
    QCOMPARE(spy.count(), 1);
    QVERIFY(spy.first().at(1).toString().contains(QStringLiteral("没有数据")));
    QCOMPARE(exporter.totalEmptySkips(), 1ULL);
}

void DataExporterErrorsTest::emptyPathEmitsExportError()
{
    DataExporter exporter;
    QSignalSpy spy(&exporter, &DataExporter::exportError);

    const bool ok = exporter.exportToFile(QString(),
                                          DataExporter::Plain,
                                          {makeLine(QDateTime::currentDateTime())});

    QVERIFY(!ok);
    QCOMPARE(spy.count(), 1);
    QVERIFY(spy.first().at(1).toString().contains(QStringLiteral("文件路径为空")));
    QCOMPARE(exporter.totalErrors(), 1ULL);
}

void DataExporterErrorsTest::whitespacePathEmitsExportError()
{
    DataExporter exporter;
    QSignalSpy spy(&exporter, &DataExporter::exportError);

    const bool ok = exporter.exportToFile(QStringLiteral("   "),
                                          DataExporter::Plain,
                                          {makeLine(QDateTime::currentDateTime())});

    QVERIFY(!ok);
    QCOMPARE(spy.count(), 1);
    QVERIFY(spy.first().at(1).toString().contains(QStringLiteral("文件路径为空")));
    QCOMPARE(exporter.totalErrors(), 1ULL);
    QCOMPARE(exporter.totalExports(), 0ULL);
}

void DataExporterErrorsTest::streamedEmptyPathEmitsExportError()
{
    DataExporter exporter;
    QSignalSpy spy(&exporter, &DataExporter::exportError);

    const bool ok = exporter.exportStreamed(QString(),
                                            DataExporter::Plain,
                                            [](int, int) { return QVector<TerminalLine>{makeLine(QDateTime::currentDateTime())}; },
                                            1);

    QVERIFY(!ok);
    QCOMPARE(spy.count(), 1);
    QVERIFY(spy.first().at(1).toString().contains(QStringLiteral("文件路径为空")));
    QCOMPARE(exporter.totalErrors(), 1ULL);
    QCOMPARE(exporter.totalExports(), 0ULL);
}

void DataExporterErrorsTest::streamedWhitespacePathEmitsExportError()
{
    DataExporter exporter;
    QSignalSpy spy(&exporter, &DataExporter::exportError);

    const bool ok = exporter.exportStreamed(QStringLiteral("   "),
                                            DataExporter::Plain,
                                            [](int, int) { return QVector<TerminalLine>{makeLine(QDateTime::currentDateTime())}; },
                                            1);

    QVERIFY(!ok);
    QCOMPARE(spy.count(), 1);
    QVERIFY(spy.first().at(1).toString().contains(QStringLiteral("文件路径为空")));
    QCOMPARE(exporter.totalErrors(), 1ULL);
    QCOMPARE(exporter.totalExports(), 0ULL);
}

void DataExporterErrorsTest::streamedEmptyInputEmitsExportError()
{
    DataExporter exporter;
    QSignalSpy spy(&exporter, &DataExporter::exportError);

    const bool ok = exporter.exportStreamed(QStringLiteral("capture.txt"),
                                            DataExporter::Plain,
                                            [](int, int) { return QVector<TerminalLine>{}; },
                                            0);

    QVERIFY(!ok);
    QCOMPARE(spy.count(), 1);
    QVERIFY(spy.first().at(1).toString().contains(QStringLiteral("没有数据")));
    QCOMPARE(exporter.totalEmptySkips(), 1ULL);
    QCOMPARE(exporter.totalExports(), 0ULL);
}

void DataExporterErrorsTest::streamedProviderEmptyBatchDoesNotReportSuccess()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    DataExporter exporter;
    QSignalSpy errorSpy(&exporter, &DataExporter::exportError);
    QSignalSpy completedSpy(&exporter, &DataExporter::exportCompleted);

    const bool ok = exporter.exportStreamed(dir.filePath(QStringLiteral("capture.txt")),
                                            DataExporter::Plain,
                                            [](int, int) { return QVector<TerminalLine>{}; },
                                            1);

    QVERIFY(!ok);
    QCOMPARE(errorSpy.count(), 1);
    QVERIFY(errorSpy.first().at(1).toString().contains(QStringLiteral("没有数据")));
    QCOMPARE(completedSpy.count(), 0);
    QCOMPARE(exporter.lastExportRowCount(), 0ULL);
    QCOMPARE(exporter.totalErrors(), 1ULL);
}

void DataExporterErrorsTest::streamedNonPositiveBatchSizeFallsBackToDefaultBatch()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    DataExporter exporter;
    QSignalSpy errorSpy(&exporter, &DataExporter::exportError);
    QSignalSpy completedSpy(&exporter, &DataExporter::exportCompleted);
    const QDateTime now = QDateTime::currentDateTime();

    const bool ok = exporter.exportStreamed(dir.filePath(QStringLiteral("capture.txt")),
                                            DataExporter::Plain,
                                            [now](int offset, int count) {
                                                if (offset > 0 || count <= 0) return QVector<TerminalLine>{};
                                                return QVector<TerminalLine>{makeLine(now)};
                                            },
                                            1,
                                            0);

    QVERIFY(ok);
    QCOMPARE(errorSpy.count(), 0);
    QCOMPARE(completedSpy.count(), 1);
    QCOMPARE(exporter.lastExportRowCount(), 1ULL);
}

void DataExporterErrorsTest::failedPlainExportDoesNotIncrementPlainExportCounter()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    DataExporter exporter;
    QSignalSpy errorSpy(&exporter, &DataExporter::exportError);

    const bool ok = exporter.exportToFile(dir.path(),
                                          DataExporter::Plain,
                                          {makeLine(QDateTime::currentDateTime())});

    QVERIFY(!ok);
    QCOMPARE(errorSpy.count(), 1);
    QCOMPARE(exporter.totalPlainExports(), 0ULL);
    QCOMPARE(exporter.totalErrors(), 1ULL);
    QCOMPARE(exporter.lastExportRowCount(), 0ULL);
}

void DataExporterErrorsTest::failedStreamedPlainExportDoesNotIncrementPlainExportCounter()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    DataExporter exporter;
    QSignalSpy errorSpy(&exporter, &DataExporter::exportError);

    const bool ok = exporter.exportStreamed(dir.path(),
                                            DataExporter::Plain,
                                            [](int, int) {
                                                return QVector<TerminalLine>{makeLine(QDateTime::currentDateTime())};
                                            },
                                            1);

    QVERIFY(!ok);
    QCOMPARE(errorSpy.count(), 1);
    QCOMPARE(exporter.totalPlainExports(), 0ULL);
    QCOMPARE(exporter.totalErrors(), 1ULL);
    QCOMPARE(exporter.lastExportRowCount(), 0ULL);
}

void DataExporterErrorsTest::failedStreamedExportDoesNotIncrementTotalExports()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    DataExporter exporter;

    const bool ok = exporter.exportStreamed(dir.path(),
                                            DataExporter::Plain,
                                            [](int, int) {
                                                return QVector<TerminalLine>{makeLine(QDateTime::currentDateTime())};
                                            },
                                            1);

    QVERIFY(!ok);
    QCOMPARE(exporter.totalExports(), 0ULL);
    QCOMPARE(exporter.totalRowsExported(), 0ULL);
    QCOMPARE(exporter.lastExportRowCount(), 0ULL);
}

void DataExporterErrorsTest::filteredOutLinesEmitExportError()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    DataExporter exporter;
    QSignalSpy spy(&exporter, &DataExporter::exportError);
    const QDateTime base = QDateTime::fromString(QStringLiteral("2026-06-11T10:00:00"),
                                                 Qt::ISODate);

    const bool ok = exporter.exportToFile(dir.filePath(QStringLiteral("capture.txt")),
                                          DataExporter::Plain,
                                          {makeLine(base)},
                                          base.addSecs(60),
                                          base.addSecs(120));

    QVERIFY(!ok);
    QCOMPARE(spy.count(), 1);
    QVERIFY(spy.first().at(1).toString().contains(QStringLiteral("时间范围")));
    QCOMPARE(exporter.totalEmptySkips(), 1ULL);
}

void DataExporterErrorsTest::failedPlainExportDoesNotIncrementTotalExports()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    DataExporter exporter;

    const bool ok = exporter.exportToFile(dir.path(),
                                          DataExporter::Plain,
                                          {makeLine(QDateTime::currentDateTime())});

    QVERIFY(!ok);
    QCOMPARE(exporter.totalExports(), 0ULL);
    QCOMPARE(exporter.totalRowsExported(), 0ULL);
    QCOMPARE(exporter.totalBytesExported(), 0ULL);
}

void DataExporterErrorsTest::filteredOutLinesDoNotIncrementTotalExports()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    DataExporter exporter;
    const QDateTime base = QDateTime::fromString(QStringLiteral("2026-06-11T10:00:00"),
                                                 Qt::ISODate);

    const bool ok = exporter.exportToFile(dir.filePath(QStringLiteral("capture.txt")),
                                          DataExporter::Plain,
                                          {makeLine(base)},
                                          base.addSecs(60),
                                          base.addSecs(120));

    QVERIFY(!ok);
    QCOMPARE(exporter.totalExports(), 0ULL);
    QCOMPARE(exporter.totalRowsExported(), 0ULL);
    QCOMPARE(exporter.totalBytesExported(), 0ULL);
}

void DataExporterErrorsTest::rangeExportEmptyPathsEmitExportError()
{
    DataExporter exporter;
    QSignalSpy spy(&exporter, &DataExporter::exportError);

    const bool ok = exporter.exportRange(QString(), DataExporter::Plain, QStringLiteral("out.txt"));

    QVERIFY(!ok);
    QCOMPARE(spy.count(), 1);
    QVERIFY(spy.first().at(1).toString().contains(QStringLiteral("文件路径为空")));
    QCOMPARE(exporter.totalErrors(), 1ULL);
    QCOMPARE(exporter.totalExports(), 0ULL);
}

void DataExporterErrorsTest::rangeExportWhitespacePathsEmitExportError()
{
    DataExporter exporter;
    QSignalSpy spy(&exporter, &DataExporter::exportError);

    const bool ok = exporter.exportRange(QStringLiteral("   "),
                                         DataExporter::Plain,
                                         QStringLiteral("   "));

    QVERIFY(!ok);
    QCOMPARE(spy.count(), 1);
    QVERIFY(spy.first().at(1).toString().contains(QStringLiteral("文件路径为空")));
    QCOMPARE(exporter.totalErrors(), 1ULL);
    QCOMPARE(exporter.totalExports(), 0ULL);
}

void DataExporterErrorsTest::rangeExportInvalidTimeRangeDoesNotIncrementTotalExports()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    DataExporter exporter;
    QSignalSpy spy(&exporter, &DataExporter::exportError);

    const bool ok = exporter.exportRange(dir.filePath(QStringLiteral("capture.edl")),
                                         DataExporter::Plain,
                                         dir.filePath(QStringLiteral("out.txt")),
                                         200,
                                         100);

    QVERIFY(!ok);
    QCOMPARE(spy.count(), 1);
    QVERIFY(spy.first().at(1).toString().contains(QStringLiteral("时间范围")));
    QCOMPARE(exporter.totalErrors(), 1ULL);
    QCOMPARE(exporter.totalExports(), 0ULL);
}

QTEST_MAIN(DataExporterErrorsTest)
#include "test_data_exporter_errors.moc"
