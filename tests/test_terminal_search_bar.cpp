#include <QtTest/QtTest>

#include <QCheckBox>
#include <QLabel>
#include <QLineEdit>
#include <QSignalSpy>

#include "terminal/search/TerminalSearchBar.h"

class TerminalSearchBarTest : public QObject {
    Q_OBJECT

private slots:
    void togglingHexWithInvalidTextDoesNotSearch();
    void validHexAfterErrorSearchesAndClearsError();
    void invalidRegexDoesNotSearchOrCount();
    void validRegexAfterErrorSearchesAndClearsError();
    void escapeInSearchInputClosesSearchBar();
    void repeatedDeactivateEmitsClosedOnce();
    void hexModeUnchecksRegexMode();
    void regexModeUnchecksHexMode();
    void matchResultShowsNotFoundForActiveSearch();
    void matchResultClearsWhenSearchIsEmpty();
    void matchResultShowsCurrentAndTotal();
};

void TerminalSearchBarTest::togglingHexWithInvalidTextDoesNotSearch()
{
    TerminalSearchBar searchBar;
    auto* input = searchBar.findChild<QLineEdit*>("searchBarInput");
    auto* hexCheck = searchBar.findChild<QCheckBox*>("searchBarHexCheck");
    auto* result = searchBar.findChild<QLabel*>("searchBarResult");
    QVERIFY(input);
    QVERIFY(hexCheck);
    QVERIFY(result);

    input->setText(QStringLiteral("not-hex"));
    QSignalSpy searchSpy(&searchBar, &TerminalSearchBar::searchRequested);

    hexCheck->setChecked(true);

    QCOMPARE(searchSpy.count(), 0);
    QCOMPARE(result->text(), QStringLiteral("非法HEX"));
    QCOMPARE(input->property("hasError").toBool(), true);
}

void TerminalSearchBarTest::validHexAfterErrorSearchesAndClearsError()
{
    TerminalSearchBar searchBar;
    auto* input = searchBar.findChild<QLineEdit*>("searchBarInput");
    auto* hexCheck = searchBar.findChild<QCheckBox*>("searchBarHexCheck");
    auto* result = searchBar.findChild<QLabel*>("searchBarResult");
    QVERIFY(input);
    QVERIFY(hexCheck);
    QVERIFY(result);

    input->setText(QStringLiteral("not-hex"));
    hexCheck->setChecked(true);
    QSignalSpy searchSpy(&searchBar, &TerminalSearchBar::searchRequested);

    input->setText(QStringLiteral("AA 55"));

    QCOMPARE(searchSpy.count(), 1);
    const auto args = searchSpy.first();
    QCOMPARE(args.at(0).toString(), QStringLiteral("AA 55"));
    QCOMPARE(args.at(2).toBool(), true);
    QCOMPARE(input->property("hasError").toBool(), false);
    QCOMPARE(result->property("hasError").toBool(), false);
}

void TerminalSearchBarTest::invalidRegexDoesNotSearchOrCount()
{
    TerminalSearchBar searchBar;
    auto* input = searchBar.findChild<QLineEdit*>("searchBarInput");
    auto* regexCheck = searchBar.findChild<QCheckBox*>("searchBarRegexCheck");
    auto* result = searchBar.findChild<QLabel*>("searchBarResult");
    QVERIFY(input);
    QVERIFY(regexCheck);
    QVERIFY(result);
    QSignalSpy searchSpy(&searchBar, &TerminalSearchBar::searchRequested);

    regexCheck->setChecked(true);
    input->setText(QStringLiteral("["));

    QCOMPARE(searchSpy.count(), 0);
    QCOMPARE(result->text(), QStringLiteral("非法正则"));
    QCOMPARE(input->property("hasError").toBool(), true);
    QCOMPARE(searchBar.totalSearches(), 0ULL);
    QCOMPARE(searchBar.totalRegexSearches(), 0ULL);
}

void TerminalSearchBarTest::validRegexAfterErrorSearchesAndClearsError()
{
    TerminalSearchBar searchBar;
    auto* input = searchBar.findChild<QLineEdit*>("searchBarInput");
    auto* regexCheck = searchBar.findChild<QCheckBox*>("searchBarRegexCheck");
    auto* result = searchBar.findChild<QLabel*>("searchBarResult");
    QVERIFY(input);
    QVERIFY(regexCheck);
    QVERIFY(result);

    regexCheck->setChecked(true);
    input->setText(QStringLiteral("["));
    QSignalSpy searchSpy(&searchBar, &TerminalSearchBar::searchRequested);

    input->setText(QStringLiteral("ERR.*"));

    QCOMPARE(searchSpy.count(), 1);
    const auto args = searchSpy.first();
    QCOMPARE(args.at(0).toString(), QStringLiteral("ERR.*"));
    QCOMPARE(args.at(1).toBool(), true);
    QCOMPARE(input->property("hasError").toBool(), false);
    QCOMPARE(result->property("hasError").toBool(), false);
    QCOMPARE(searchBar.totalRegexSearches(), 1ULL);
}

void TerminalSearchBarTest::escapeInSearchInputClosesSearchBar()
{
    TerminalSearchBar searchBar;
    auto* input = searchBar.findChild<QLineEdit*>("searchBarInput");
    QVERIFY(input);

    QSignalSpy closedSpy(&searchBar, &TerminalSearchBar::closed);
    searchBar.activate();
    QVERIFY(searchBar.isVisible());
    QVERIFY(input->hasFocus());

    QTest::keyClick(input, Qt::Key_Escape);

    QVERIFY(closedSpy.wait(500));
    QCOMPARE(closedSpy.count(), 1);
    QVERIFY(!searchBar.isVisible());
}

void TerminalSearchBarTest::repeatedDeactivateEmitsClosedOnce()
{
    TerminalSearchBar searchBar;
    QSignalSpy closedSpy(&searchBar, &TerminalSearchBar::closed);

    searchBar.activate();
    QVERIFY(searchBar.isVisible());
    searchBar.deactivate();
    searchBar.deactivate();

    QVERIFY(closedSpy.wait(500));
    QTest::qWait(250);
    QCOMPARE(closedSpy.count(), 1);
}

void TerminalSearchBarTest::hexModeUnchecksRegexMode()
{
    TerminalSearchBar searchBar;
    auto* input = searchBar.findChild<QLineEdit*>("searchBarInput");
    auto* regexCheck = searchBar.findChild<QCheckBox*>("searchBarRegexCheck");
    auto* hexCheck = searchBar.findChild<QCheckBox*>("searchBarHexCheck");
    QVERIFY(input);
    QVERIFY(regexCheck);
    QVERIFY(hexCheck);

    input->setText(QStringLiteral("AA 55"));
    regexCheck->setChecked(true);
    QVERIFY(searchBar.isRegexMode());

    hexCheck->setChecked(true);

    QVERIFY(searchBar.isHexMode());
    QVERIFY(!searchBar.isRegexMode());
}

void TerminalSearchBarTest::regexModeUnchecksHexMode()
{
    TerminalSearchBar searchBar;
    auto* input = searchBar.findChild<QLineEdit*>("searchBarInput");
    auto* regexCheck = searchBar.findChild<QCheckBox*>("searchBarRegexCheck");
    auto* hexCheck = searchBar.findChild<QCheckBox*>("searchBarHexCheck");
    QVERIFY(input);
    QVERIFY(regexCheck);
    QVERIFY(hexCheck);

    input->setText(QStringLiteral("AA 55"));
    hexCheck->setChecked(true);
    QVERIFY(searchBar.isHexMode());

    regexCheck->setChecked(true);

    QVERIFY(searchBar.isRegexMode());
    QVERIFY(!searchBar.isHexMode());
}

void TerminalSearchBarTest::matchResultShowsNotFoundForActiveSearch()
{
    TerminalSearchBar searchBar;
    auto* input = searchBar.findChild<QLineEdit*>("searchBarInput");
    auto* resultLabel = searchBar.findChild<QLabel*>("searchBarResult");
    QVERIFY(input);
    QVERIFY(resultLabel);

    input->setText(QStringLiteral("missing"));
    searchBar.setMatchResult(0, -1);

    QCOMPARE(resultLabel->text(), QStringLiteral("未找到"));
}

void TerminalSearchBarTest::matchResultClearsWhenSearchIsEmpty()
{
    TerminalSearchBar searchBar;
    auto* resultLabel = searchBar.findChild<QLabel*>("searchBarResult");
    QVERIFY(resultLabel);

    searchBar.setResultText(QStringLiteral("未找到"));
    searchBar.setMatchResult(0, -1);

    QVERIFY(resultLabel->text().isEmpty());
}

void TerminalSearchBarTest::matchResultShowsCurrentAndTotal()
{
    TerminalSearchBar searchBar;
    auto* input = searchBar.findChild<QLineEdit*>("searchBarInput");
    auto* resultLabel = searchBar.findChild<QLabel*>("searchBarResult");
    QVERIFY(input);
    QVERIFY(resultLabel);

    input->setText(QStringLiteral("hit"));
    searchBar.setMatchResult(5, 2);

    QCOMPARE(resultLabel->text(), QStringLiteral("3/5"));
}

QTEST_MAIN(TerminalSearchBarTest)
#include "test_terminal_search_bar.moc"
