#include <QtTest/QtTest>

#include "core/recording/DataStreamRecorder.h"

#include <QSignalSpy>
#include <QTemporaryDir>

class DataStreamRecorderTest : public QObject {
    Q_OBJECT

private slots:
    void whitespaceOnlyPathDoesNotStartRecording();
    void validPathStartsAndCreatesExpectedFile();
    void whitespaceOnlyAnnotationDoesNotPolluteRecording();
    void whitespaceOnlyMarkerDoesNotPolluteRecording();
};

void DataStreamRecorderTest::whitespaceOnlyPathDoesNotStartRecording()
{
    DataStreamRecorder recorder;
    DataStreamRecorder::RecordingConfig config;
    config.filePath = QStringLiteral("   ");
    QSignalSpy errorSpy(&recorder, &DataStreamRecorder::errorOccurred);
    QSignalSpy startedSpy(&recorder, &DataStreamRecorder::recordingStarted);

    const bool ok = recorder.start(config);

    QVERIFY(!ok);
    QVERIFY(!recorder.isRecording());
    QCOMPARE(errorSpy.count(), 1);
    QVERIFY(errorSpy.first().first().toString().contains(QStringLiteral("路径不能为空")));
    QCOMPARE(startedSpy.count(), 0);
    QCOMPARE(recorder.stats().totalRecordingSessions, 0ULL);
    QCOMPARE(recorder.stats().totalFilesCreated, 0ULL);
}

void DataStreamRecorderTest::validPathStartsAndCreatesExpectedFile()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    DataStreamRecorder recorder;
    DataStreamRecorder::RecordingConfig config;
    config.filePath = dir.filePath(QStringLiteral("capture.csv"));
    config.format = DataStreamRecorder::FileFormat::TimestampedCsv;
    QSignalSpy startedSpy(&recorder, &DataStreamRecorder::recordingStarted);

    QVERIFY(recorder.start(config));
    QVERIFY(recorder.isRecording());
    QCOMPARE(startedSpy.count(), 1);
    const QString createdPath = startedSpy.first().first().toString();
    QVERIFY(QFile::exists(createdPath));
    recorder.stop();

    QCOMPARE(recorder.stats().totalRecordingSessions, 1ULL);
    QCOMPARE(recorder.stats().totalFilesCreated, 1ULL);
}

void DataStreamRecorderTest::whitespaceOnlyAnnotationDoesNotPolluteRecording()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    DataStreamRecorder recorder;
    DataStreamRecorder::RecordingConfig config;
    config.filePath = dir.filePath(QStringLiteral("capture.csv"));
    config.format = DataStreamRecorder::FileFormat::TimestampedCsv;
    QSignalSpy annotationSpy(&recorder, &DataStreamRecorder::annotationAdded);

    QVERIFY(recorder.start(config));
    recorder.addAnnotation(QStringLiteral("   "));
    recorder.stop();

    QCOMPARE(annotationSpy.count(), 0);
    QCOMPARE(recorder.recordingStats().totalAnnotations, 0ULL);
}

void DataStreamRecorderTest::whitespaceOnlyMarkerDoesNotPolluteRecording()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    DataStreamRecorder recorder;
    DataStreamRecorder::RecordingConfig config;
    config.filePath = dir.filePath(QStringLiteral("capture.txt"));
    config.format = DataStreamRecorder::FileFormat::HexDump;
    QSignalSpy annotationSpy(&recorder, &DataStreamRecorder::annotationAdded);

    QVERIFY(recorder.start(config));
    recorder.addMarker(QStringLiteral("   "));
    recorder.stop();

    QCOMPARE(annotationSpy.count(), 0);
    QCOMPARE(recorder.recordingStats().totalAnnotations, 0ULL);
}

QTEST_MAIN(DataStreamRecorderTest)
#include "test_data_stream_recorder.moc"
