#include <QtTest/QtTest>

#include "core/send/SendHistoryManager.h"
#include "serial/commands/SendHistory.h"

class SendHistoryManagerTest : public QObject {
    Q_OBJECT

private slots:
    void recordHistoryIgnoresWhitespaceOnlyTextForStats();
    void recordHistoryCountsOnlyWhenHistoryActuallyGrows();
};

void SendHistoryManagerTest::recordHistoryIgnoresWhitespaceOnlyTextForStats()
{
    SendHistory history;
    SendHistoryManager manager(&history);

    manager.recordHistory(QStringLiteral("   "), false);

    QVERIFY(history.entries().isEmpty());
    QCOMPARE(manager.totalAdds(), 0ULL);
    QCOMPARE(manager.peakHistorySize(), 0ULL);
}

void SendHistoryManagerTest::recordHistoryCountsOnlyWhenHistoryActuallyGrows()
{
    SendHistory history;
    SendHistoryManager manager(&history);

    manager.recordHistory(QStringLiteral(" AT "), false);
    manager.recordHistory(QStringLiteral("AT"), false);

    QCOMPARE(history.entries().size(), 1);
    QCOMPARE(manager.totalAdds(), 1ULL);
    QCOMPARE(manager.peakHistorySize(), 1ULL);
}

QTEST_MAIN(SendHistoryManagerTest)
#include "test_send_history_manager.moc"
