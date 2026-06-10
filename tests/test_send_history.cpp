#include <QtTest/QtTest>

#include "serial/commands/SendHistory.h"

#include <QSignalSpy>

class SendHistoryTest : public QObject {
    Q_OBJECT

private slots:
    void recentTextsDeduplicateNonConsecutiveCommandsByRecency();
    void recentTextsLimitCountsUniqueCommands();
    void addEntryIgnoresWhitespaceOnlyText();
    void addEntryTrimsTextForHistoryAndDuplicateDetection();
};

void SendHistoryTest::recentTextsDeduplicateNonConsecutiveCommandsByRecency()
{
    SendHistory history;
    history.addEntry(QStringLiteral("AT"), false);
    history.addEntry(QStringLiteral("RESET"), false);
    history.addEntry(QStringLiteral("AT"), false);

    const QStringList recent = history.recentTexts();

    QCOMPARE(recent, QStringList({QStringLiteral("AT"), QStringLiteral("RESET")}));
}

void SendHistoryTest::recentTextsLimitCountsUniqueCommands()
{
    SendHistory history;
    history.addEntry(QStringLiteral("AT"), false);
    history.addEntry(QStringLiteral("STATUS"), false);
    history.addEntry(QStringLiteral("RESET"), false);
    history.addEntry(QStringLiteral("STATUS"), false);

    const QStringList recent = history.recentTexts(2);

    QCOMPARE(recent, QStringList({QStringLiteral("STATUS"), QStringLiteral("RESET")}));
}

void SendHistoryTest::addEntryIgnoresWhitespaceOnlyText()
{
    SendHistory history;
    QSignalSpy changedSpy(&history, &SendHistory::historyChanged);

    history.addEntry(QStringLiteral("   "), false);

    QVERIFY(history.entries().isEmpty());
    QVERIFY(history.recentTexts().isEmpty());
    QCOMPARE(history.totalRecords(), 0ULL);
    QCOMPARE(changedSpy.count(), 0);
}

void SendHistoryTest::addEntryTrimsTextForHistoryAndDuplicateDetection()
{
    SendHistory history;
    QSignalSpy changedSpy(&history, &SendHistory::historyChanged);

    history.addEntry(QStringLiteral("  AT  "), false);
    history.addEntry(QStringLiteral("AT"), false);

    QCOMPARE(history.entries().size(), 1);
    QCOMPARE(history.entries().first().text, QStringLiteral("AT"));
    QCOMPARE(history.recentTexts(), QStringList{QStringLiteral("AT")});
    QCOMPARE(history.totalDuplicateSkips(), 1ULL);
    QCOMPARE(changedSpy.count(), 1);
}

QTEST_MAIN(SendHistoryTest)
#include "test_send_history.moc"
