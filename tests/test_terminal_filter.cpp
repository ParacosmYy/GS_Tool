#include <QtTest/QtTest>

#include "terminal/filter/TerminalFilter.h"

class TerminalFilterTest : public QObject {
    Q_OBJECT

private slots:
    void singlePatternTrimsWhitespaceBeforeMatching();
    void addRuleTrimsWhitespaceBeforeFiltering();
    void updateRuleTrimsWhitespaceBeforeFiltering();
    void whitespaceOnlyPatternIsInvalid();
};

void TerminalFilterTest::singlePatternTrimsWhitespaceBeforeMatching()
{
    TerminalFilter filter;

    QVERIFY(filter.setPattern(QStringLiteral("  ERROR  ")));

    QVERIFY(filter.match(QStringLiteral("ERROR: timeout")));
    QCOMPARE(filter.matchCount(), 1ULL);
}

void TerminalFilterTest::addRuleTrimsWhitespaceBeforeFiltering()
{
    TerminalFilter filter;

    const int index = filter.addRule(QStringLiteral("  ERROR  "));

    QCOMPARE(index, 0);
    QVERIFY(filter.filter(QStringLiteral("ERROR: timeout")));
    QCOMPARE(filter.rules().first().pattern, QStringLiteral("ERROR"));
}

void TerminalFilterTest::updateRuleTrimsWhitespaceBeforeFiltering()
{
    TerminalFilter filter;
    const int index = filter.addRule(QStringLiteral("WARN"));
    QCOMPARE(index, 0);

    QVERIFY(filter.updateRule(index, QStringLiteral("  ERROR  ")));

    QVERIFY(filter.filter(QStringLiteral("ERROR: timeout")));
    QCOMPARE(filter.rules().first().pattern, QStringLiteral("ERROR"));
}

void TerminalFilterTest::whitespaceOnlyPatternIsInvalid()
{
    TerminalFilter filter;

    QVERIFY(!filter.setPattern(QStringLiteral("   ")));
    QCOMPARE(filter.addRule(QStringLiteral("   ")), -1);
    QVERIFY(!filter.match(QStringLiteral("anything")));
}

QTEST_MAIN(TerminalFilterTest)
#include "test_terminal_filter.moc"
