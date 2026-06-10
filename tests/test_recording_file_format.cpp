#include <QtTest/QtTest>

#include "utils/log/RecordingFileFormat.h"

#include <QTemporaryDir>

class RecordingFileFormatTest : public QObject {
    Q_OBJECT

private slots:
    void saveWhitespacePathReportsClearError();
    void loadWhitespacePathReportsClearError();
    void saveAndLoadRoundTrip();
};

void RecordingFileFormatTest::saveWhitespacePathReportsClearError()
{
    RecordingFileFormat format;
    QVERIFY(format.setData(QByteArray("abc"), {}));

    QVERIFY(!format.saveToFile(QStringLiteral("   ")));

    QVERIFY(format.lastError().contains(QStringLiteral("路径不能为空")));
    QCOMPARE(format.totalSaves(), 0ULL);
    QCOMPARE(format.totalErrors(), 1ULL);
    QCOMPARE(format.serializationErrors(), 1ULL);
}

void RecordingFileFormatTest::loadWhitespacePathReportsClearError()
{
    RecordingFileFormat format;

    QVERIFY(!format.loadFromFile(QStringLiteral("   ")));

    QVERIFY(format.lastError().contains(QStringLiteral("路径不能为空")));
    QCOMPARE(format.totalLoads(), 0ULL);
    QCOMPARE(format.totalErrors(), 1ULL);
    QCOMPARE(format.deserializationErrors(), 1ULL);
}

void RecordingFileFormatTest::saveAndLoadRoundTrip()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("capture.edb"));

    RecordingFileFormat writer;
    QVariantMap meta;
    meta.insert(QStringLiteral("name"), QStringLiteral("session"));
    QVERIFY(writer.setData(QByteArray("abc"), meta));
    QVERIFY(writer.saveToFile(path));

    RecordingFileFormat reader;
    QVERIFY(reader.loadFromFile(path));

    QCOMPARE(reader.rawData(), QByteArray("abc"));
    QCOMPARE(reader.metadata().value(QStringLiteral("name")).toString(), QStringLiteral("session"));
    QCOMPARE(writer.totalSaves(), 1ULL);
    QCOMPARE(reader.totalLoads(), 1ULL);
}

QTEST_MAIN(RecordingFileFormatTest)
#include "test_recording_file_format.moc"
