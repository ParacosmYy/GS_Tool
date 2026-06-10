#include <QtTest/QtTest>

#include "utils/log/DataLogger.h"

#include <QSignalSpy>

class DataLoggerSessionTest : public QObject {
    Q_OBJECT

private slots:
    void whitespaceRecordingPathDoesNotCreateFile();
    void whitespacePlaybackPathDoesNotStartPlayback();
    void emptyLogDataDoesNotCreateRecord();
    void firstPlaybackRecordCountsPlayedBackBytes();
    void invalidBookmarkSeekEmitsErrorAndCountsFailure();
    void addBookmarkTrimsBlankLabelToReadableFallback();
    void invalidBookmarkRemoveEmitsErrorAndCountsFailure();
    void validBookmarkRemoveKeepsTotalBookmarkHistory();
};

void DataLoggerSessionTest::whitespaceRecordingPathDoesNotCreateFile()
{
    DataLogger logger;
    QSignalSpy errorSpy(&logger, &DataLogger::error);
    QSignalSpy startedSpy(&logger, &DataLogger::recordingStarted);

    const bool ok = logger.startRecording(QStringLiteral("   "));

    QVERIFY(!ok);
    QVERIFY(!logger.isRecording());
    QCOMPARE(errorSpy.count(), 1);
    QVERIFY(errorSpy.first().first().toString().contains(QStringLiteral("路径不能为空")));
    QCOMPARE(startedSpy.count(), 0);
    QCOMPARE(logger.totalErrors(), 1ULL);
    QCOMPARE(logger.totalRecordStarts(), 0ULL);
}

void DataLoggerSessionTest::whitespacePlaybackPathDoesNotStartPlayback()
{
    DataLogger logger;
    QSignalSpy errorSpy(&logger, &DataLogger::error);

    const bool ok = logger.startPlayback(QStringLiteral("   "));

    QVERIFY(!ok);
    QVERIFY(!logger.isPlaying());
    QCOMPARE(errorSpy.count(), 1);
    QVERIFY(errorSpy.first().first().toString().contains(QStringLiteral("路径不能为空")));
    QCOMPARE(logger.totalErrors(), 1ULL);
    QCOMPARE(logger.totalPlaybacks(), 0ULL);
}

void DataLoggerSessionTest::emptyLogDataDoesNotCreateRecord()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    DataLogger logger;
    const QString path = dir.filePath(QStringLiteral("capture.edl"));

    QVERIFY(logger.startRecording(path));
    logger.logData(QByteArray(), DataLogger::Direction::Received);
    logger.stopRecording();

    QCOMPARE(logger.recordCount(), 0);
    QCOMPARE(logger.totalLogsWritten(), 0ULL);
    QCOMPARE(logger.totalRecords(), 0ULL);
    QCOMPARE(logger.totalBytesRecorded(), 0ULL);
}

void DataLoggerSessionTest::firstPlaybackRecordCountsPlayedBackBytes()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    DataLogger logger;
    const QString path = dir.filePath(QStringLiteral("capture.edl"));
    const QByteArray payload("abc");

    QVERIFY(logger.startRecording(path));
    logger.logData(payload, DataLogger::Direction::Received);
    logger.stopRecording();

    QSignalSpy playbackSpy(&logger, &DataLogger::playbackData);

    QVERIFY(logger.startPlayback(path));

    QCOMPARE(playbackSpy.count(), 1);
    QCOMPARE(playbackSpy.first().at(0).toByteArray(), payload);
    QCOMPARE(logger.totalBytesPlayedBack(), static_cast<quint64>(payload.size()));
}

void DataLoggerSessionTest::invalidBookmarkSeekEmitsErrorAndCountsFailure()
{
    DataLogger logger;
    QSignalSpy errorSpy(&logger, &DataLogger::error);

    const bool ok = logger.seekToBookmark(0);

    QVERIFY(!ok);
    QCOMPARE(errorSpy.count(), 1);
    QVERIFY(errorSpy.first().first().toString().contains(QStringLiteral("书签")));
    QCOMPARE(logger.totalErrors(), 1ULL);
    QCOMPARE(logger.totalSeeks(), 0ULL);
}

void DataLoggerSessionTest::addBookmarkTrimsBlankLabelToReadableFallback()
{
    DataLogger logger;
    QSignalSpy changedSpy(&logger, &DataLogger::bookmarksChanged);

    logger.addBookmark(QStringLiteral("   "));

    QCOMPARE(changedSpy.count(), 1);
    QCOMPARE(logger.bookmarks().size(), 1);
    QVERIFY(!logger.bookmarks().first().label.trimmed().isEmpty());
    QCOMPARE(logger.totalBookmarks(), 1ULL);
}

void DataLoggerSessionTest::invalidBookmarkRemoveEmitsErrorAndCountsFailure()
{
    DataLogger logger;
    QSignalSpy errorSpy(&logger, &DataLogger::error);
    QSignalSpy changedSpy(&logger, &DataLogger::bookmarksChanged);

    logger.removeBookmark(0);

    QCOMPARE(errorSpy.count(), 1);
    QVERIFY(errorSpy.first().first().toString().contains(QStringLiteral("书签")));
    QCOMPARE(changedSpy.count(), 0);
    QCOMPARE(logger.totalErrors(), 1ULL);
}

void DataLoggerSessionTest::validBookmarkRemoveKeepsTotalBookmarkHistory()
{
    DataLogger logger;
    QSignalSpy changedSpy(&logger, &DataLogger::bookmarksChanged);

    logger.addBookmark(QStringLiteral("first"));
    logger.removeBookmark(0);

    QCOMPARE(changedSpy.count(), 2);
    QVERIFY(logger.bookmarks().isEmpty());
    QCOMPARE(logger.totalBookmarks(), 1ULL);
    QCOMPARE(logger.totalErrors(), 0ULL);
}

QTEST_MAIN(DataLoggerSessionTest)
#include "test_data_logger_session.moc"
