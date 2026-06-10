#include <QtTest/QtTest>

#include "serial/commands/SendHistory.h"

class SendHistoryTest : public QObject {
    Q_OBJECT

private slots:
    void recentTextsDeduplicateNonConsecutiveCommandsByRecency();
    void recentTextsLimitCountsUniqueCommands();
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

QTEST_MAIN(SendHistoryTest)
#include "test_send_history.moc"
