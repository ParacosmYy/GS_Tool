#include <QtTest/QtTest>

#include "utils/export/ChartExporter.h"

#include <QSignalSpy>
#include <QWidget>

class ChartExporterErrorsTest : public QObject {
    Q_OBJECT

private slots:
    void csvWhitespacePathReportsClearError();
    void jsonWhitespacePathReportsClearError();
    void pngWhitespacePathReportsClearError();
    void svgWhitespacePathReportsClearError();
    void svgOpenFailureDoesNotReportSuccess();
};

void ChartExporterErrorsTest::csvWhitespacePathReportsClearError()
{
    ChartExporter exporter;
    QSignalSpy spy(&exporter, &ChartExporter::exportFailed);

    const bool ok = exporter.exportToCsv(QStringLiteral("   "),
                                         {QStringLiteral("CH1")},
                                         {{1.0, 2.0}});

    QVERIFY(!ok);
    QCOMPARE(spy.count(), 1);
    QVERIFY(spy.first().first().toString().contains(QStringLiteral("文件路径为空")));
    QCOMPARE(exporter.totalErrors(), 1ULL);
    QCOMPARE(exporter.totalExports(), 0ULL);
    QCOMPARE(exporter.totalExportsCsv(), 0ULL);
}

void ChartExporterErrorsTest::jsonWhitespacePathReportsClearError()
{
    ChartExporter exporter;
    QSignalSpy spy(&exporter, &ChartExporter::exportFailed);

    const bool ok = exporter.exportToJson(QStringLiteral("   "),
                                          {QStringLiteral("CH1")},
                                          {{1.0, 2.0}});

    QVERIFY(!ok);
    QCOMPARE(spy.count(), 1);
    QVERIFY(spy.first().first().toString().contains(QStringLiteral("文件路径为空")));
    QCOMPARE(exporter.totalErrors(), 1ULL);
    QCOMPARE(exporter.totalExports(), 0ULL);
    QCOMPARE(exporter.totalExportsJson(), 0ULL);
}

void ChartExporterErrorsTest::pngWhitespacePathReportsClearError()
{
    QWidget widget;
    widget.resize(32, 32);
    ChartExporter exporter;
    QSignalSpy spy(&exporter, &ChartExporter::exportFailed);

    const bool ok = exporter.exportToPng(QStringLiteral("   "), &widget);

    QVERIFY(!ok);
    QCOMPARE(spy.count(), 1);
    QVERIFY(spy.first().first().toString().contains(QStringLiteral("文件路径为空")));
    QCOMPARE(exporter.totalErrors(), 1ULL);
    QCOMPARE(exporter.totalExports(), 0ULL);
    QCOMPARE(exporter.totalExportsPng(), 0ULL);
    QCOMPARE(exporter.totalChartImages(), 0ULL);
}

void ChartExporterErrorsTest::svgWhitespacePathReportsClearError()
{
    QWidget widget;
    widget.resize(32, 32);
    ChartExporter exporter;
    QSignalSpy spy(&exporter, &ChartExporter::exportFailed);

    const bool ok = exporter.exportToSvg(QStringLiteral("   "), &widget);

    QVERIFY(!ok);
    QCOMPARE(spy.count(), 1);
    QVERIFY(spy.first().first().toString().contains(QStringLiteral("文件路径为空")));
    QCOMPARE(exporter.totalErrors(), 1ULL);
    QCOMPARE(exporter.totalExports(), 0ULL);
    QCOMPARE(exporter.totalExportsSvg(), 0ULL);
    QCOMPARE(exporter.totalChartImages(), 0ULL);
}

void ChartExporterErrorsTest::svgOpenFailureDoesNotReportSuccess()
{
    QWidget widget;
    widget.resize(32, 32);
    ChartExporter exporter;
    QSignalSpy failedSpy(&exporter, &ChartExporter::exportFailed);
    QSignalSpy completedSpy(&exporter, &ChartExporter::exportCompleted);

    const bool ok = exporter.exportToSvg(QStringLiteral("Z:/path/that/does/not/exist/chart.svg"),
                                         &widget);

    QVERIFY(!ok);
    QCOMPARE(failedSpy.count(), 1);
    QCOMPARE(completedSpy.count(), 0);
    QCOMPARE(exporter.totalErrors(), 1ULL);
    QCOMPARE(exporter.totalExports(), 0ULL);
    QCOMPARE(exporter.totalExportsSvg(), 0ULL);
    QCOMPARE(exporter.totalChartImages(), 0ULL);
}

QTEST_MAIN(ChartExporterErrorsTest)
#include "test_chart_exporter_errors.moc"
