#include <QtTest/QtTest>

#include "serial/commands/TimedSender.h"

#include <QSignalSpy>

class TimedSenderTest : public QObject {
    Q_OBJECT

private slots:
    void emptySingleDataDoesNotStartTimer();
    void emptyDataQueueDoesNotStartTimer();
    void dataQueueSkipsEmptyItems();
};

void TimedSenderTest::emptySingleDataDoesNotStartTimer()
{
    TimedSender sender;
    sender.setInterval(1);
    sender.setData(QByteArray());
    QSignalSpy sentSpy(&sender, &TimedSender::sendData);

    sender.start();
    QTest::qWait(20);

    QVERIFY(!sender.isRunning());
    QCOMPARE(sender.scheduleCount(), 0ULL);
    QCOMPARE(sender.sendCount(), 0);
    QCOMPARE(sender.totalTimedSends(), 0ULL);
    QCOMPARE(sentSpy.count(), 0);
}

void TimedSenderTest::emptyDataQueueDoesNotStartTimer()
{
    TimedSender sender;
    sender.setInterval(1);
    sender.setDataQueue({QByteArray(), QByteArray()});
    QSignalSpy sentSpy(&sender, &TimedSender::sendData);

    sender.start();
    QTest::qWait(20);

    QVERIFY(!sender.isRunning());
    QCOMPARE(sender.scheduleCount(), 0ULL);
    QCOMPARE(sender.sendCount(), 0);
    QCOMPARE(sender.totalTimedSends(), 0ULL);
    QCOMPARE(sentSpy.count(), 0);
}

void TimedSenderTest::dataQueueSkipsEmptyItems()
{
    TimedSender sender;
    sender.setInterval(5);
    sender.setDataQueue({QByteArray("AA"), QByteArray(), QByteArray("55")});
    QSignalSpy sentSpy(&sender, &TimedSender::sendData);

    sender.start();
    QTRY_VERIFY_WITH_TIMEOUT(sentSpy.count() >= 2, 80);
    sender.stop();

    QVERIFY(sender.scheduleCount() == 1ULL);
    QVERIFY(sender.sendCount() >= 2);
    for (const auto& arguments : sentSpy) {
        const QByteArray data = arguments.at(0).toByteArray();
        QVERIFY(!data.isEmpty());
        QVERIFY(data == QByteArray("AA") || data == QByteArray("55"));
    }
}

QTEST_MAIN(TimedSenderTest)
#include "test_timed_sender.moc"
