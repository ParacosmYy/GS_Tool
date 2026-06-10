#include <QtTest/QtTest>

#include "core/terminal/TerminalExportOptions.h"

class TerminalExportOptionsTest : public QObject {
    Q_OBJECT

private slots:
    void keepsExplicitCsvExtensionAndFormat();
    void appendsCsvExtensionFromSelectedFilter();
    void appendsTxtExtensionFromTextFilter();
    void appendsBinExtensionFromBinaryFilter();
    void explicitExtensionWinsOverSelectedFilter();
    void unknownExtensionDefaultsToPlainWithoutChangingPath();
};

void TerminalExportOptionsTest::keepsExplicitCsvExtensionAndFormat()
{
    const auto result = normalizedTerminalExportOptions(
        QStringLiteral("capture.csv"),
        QStringLiteral("CSV文件 (*.csv)"));

    QCOMPARE(result.filePath, QStringLiteral("capture.csv"));
    QCOMPARE(result.format, DataExporter::Csv);
}

void TerminalExportOptionsTest::appendsCsvExtensionFromSelectedFilter()
{
    const auto result = normalizedTerminalExportOptions(
        QStringLiteral("capture"),
        QStringLiteral("CSV文件 (*.csv)"));

    QCOMPARE(result.filePath, QStringLiteral("capture.csv"));
    QCOMPARE(result.format, DataExporter::Csv);
}

void TerminalExportOptionsTest::appendsTxtExtensionFromTextFilter()
{
    const auto result = normalizedTerminalExportOptions(
        QStringLiteral("capture"),
        QStringLiteral("文本文件 (*.txt)"));

    QCOMPARE(result.filePath, QStringLiteral("capture.txt"));
    QCOMPARE(result.format, DataExporter::Plain);
}

void TerminalExportOptionsTest::appendsBinExtensionFromBinaryFilter()
{
    const auto result = normalizedTerminalExportOptions(
        QStringLiteral("capture"),
        QStringLiteral("二进制文件 (*.bin)"));

    QCOMPARE(result.filePath, QStringLiteral("capture.bin"));
    QCOMPARE(result.format, DataExporter::Bin);
}

void TerminalExportOptionsTest::explicitExtensionWinsOverSelectedFilter()
{
    const auto result = normalizedTerminalExportOptions(
        QStringLiteral("capture.bin"),
        QStringLiteral("CSV文件 (*.csv)"));

    QCOMPARE(result.filePath, QStringLiteral("capture.bin"));
    QCOMPARE(result.format, DataExporter::Bin);
}

void TerminalExportOptionsTest::unknownExtensionDefaultsToPlainWithoutChangingPath()
{
    const auto result = normalizedTerminalExportOptions(
        QStringLiteral("capture.log"),
        QStringLiteral("CSV文件 (*.csv)"));

    QCOMPARE(result.filePath, QStringLiteral("capture.log"));
    QCOMPARE(result.format, DataExporter::Plain);
}

QTEST_MAIN(TerminalExportOptionsTest)
#include "test_terminal_export_options.moc"
