#include <QtTest/QtTest>

#include "terminal/search/TerminalSearchManager.h"

#include <QSignalSpy>

class TerminalSearchManagerTest : public QObject {
    Q_OBJECT

private slots:
    void invalidRegexDoesNotPolluteHistoryOrSearchStats();
    void invalidHexDoesNotPolluteHistoryOrSearchStats();
    void emptyPatternClearsSearchWithoutCountingError();
    void emptyTerminalReturnsNoMatchesWithoutCountingError();
    void validSearchAddsHistoryAndCountsSearch();
};

namespace {

QVector<CachedLine> cachedLines()
{
    CachedLine line;
    line.text = QStringLiteral("ERROR: device timeout");
    line.direction = DataDirection::Rx;
    line.timestamp = 0;
    return {line};
}

QByteArray lineBytes(int)
{
    return QByteArray::fromHex("AA5501");
}

} // namespace

void TerminalSearchManagerTest::invalidRegexDoesNotPolluteHistoryOrSearchStats()
{
    TerminalSearchManager manager;
    QSignalSpy historySpy(&manager, &TerminalSearchManager::searchHistoryChanged);
    QSignalSpy matchesSpy(&manager, &TerminalSearchManager::searchMatchesChanged);

    const int count = manager.setSearchHighlight(QStringLiteral("["),
                                                 true,
                                                 false,
                                                 false,
                                                 false,
                                                 cachedLines(),
                                                 nullptr,
                                                 1,
                                                 lineBytes);

    QCOMPARE(count, 0);
    QVERIFY(manager.searchHistory().isEmpty());
    QCOMPARE(manager.totalSearches(), 0ULL);
    QCOMPARE(manager.searchErrorCount(), 1ULL);
    QCOMPARE(historySpy.count(), 0);
    QCOMPARE(matchesSpy.count(), 1);
}

void TerminalSearchManagerTest::invalidHexDoesNotPolluteHistoryOrSearchStats()
{
    TerminalSearchManager manager;
    QSignalSpy historySpy(&manager, &TerminalSearchManager::searchHistoryChanged);

    const int count = manager.setSearchHighlight(QStringLiteral("AA 5"),
                                                 false,
                                                 true,
                                                 false,
                                                 false,
                                                 cachedLines(),
                                                 nullptr,
                                                 1,
                                                 lineBytes);

    QCOMPARE(count, 0);
    QVERIFY(manager.searchHistory().isEmpty());
    QCOMPARE(manager.totalSearches(), 0ULL);
    QCOMPARE(manager.searchErrorCount(), 1ULL);
    QCOMPARE(historySpy.count(), 0);
}

void TerminalSearchManagerTest::emptyPatternClearsSearchWithoutCountingError()
{
    TerminalSearchManager manager;
    manager.setSearchHighlight(QStringLiteral("ERROR"),
                               false,
                               false,
                               false,
                               false,
                               cachedLines(),
                               nullptr,
                               1,
                               lineBytes);
    QCOMPARE(manager.searchMatchCount(), 1);

    QSignalSpy matchesSpy(&manager, &TerminalSearchManager::searchMatchesChanged);
    const int count = manager.setSearchHighlight(QString(),
                                                 false,
                                                 false,
                                                 false,
                                                 false,
                                                 cachedLines(),
                                                 nullptr,
                                                 1,
                                                 lineBytes);

    QCOMPARE(count, 0);
    QCOMPARE(manager.searchMatchCount(), 0);
    QCOMPARE(manager.currentMatchIndex(), -1);
    QCOMPARE(manager.searchErrorCount(), 0ULL);
    QCOMPARE(manager.totalSearches(), 1ULL);
    QCOMPARE(manager.searchHistory(), QStringList{QStringLiteral("ERROR")});
    QCOMPARE(matchesSpy.count(), 1);
}

void TerminalSearchManagerTest::emptyTerminalReturnsNoMatchesWithoutCountingError()
{
    TerminalSearchManager manager;
    QSignalSpy matchesSpy(&manager, &TerminalSearchManager::searchMatchesChanged);

    const int count = manager.setSearchHighlight(QStringLiteral("ERROR"),
                                                 false,
                                                 false,
                                                 false,
                                                 false,
                                                 {},
                                                 nullptr,
                                                 0,
                                                 lineBytes);

    QCOMPARE(count, 0);
    QCOMPARE(manager.searchMatchCount(), 0);
    QCOMPARE(manager.searchErrorCount(), 0ULL);
    QCOMPARE(manager.totalSearches(), 0ULL);
    QVERIFY(manager.searchHistory().isEmpty());
    QCOMPARE(matchesSpy.count(), 1);
}

void TerminalSearchManagerTest::validSearchAddsHistoryAndCountsSearch()
{
    TerminalSearchManager manager;
    QSignalSpy historySpy(&manager, &TerminalSearchManager::searchHistoryChanged);

    const int count = manager.setSearchHighlight(QStringLiteral("ERROR"),
                                                 false,
                                                 false,
                                                 false,
                                                 false,
                                                 cachedLines(),
                                                 nullptr,
                                                 1,
                                                 lineBytes);

    QCOMPARE(count, 1);
    QCOMPARE(manager.searchHistory(), QStringList{QStringLiteral("ERROR")});
    QCOMPARE(manager.totalSearches(), 1ULL);
    QCOMPARE(manager.totalMatches(), 1ULL);
    QCOMPARE(historySpy.count(), 1);
}

QTEST_MAIN(TerminalSearchManagerTest)
#include "test_terminal_search_manager.moc"
