#include <QtTest/QtTest>

#include "terminal/filter/TerminalFilterBar.h"

#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QSignalSpy>

class TerminalFilterBarTest : public QObject {
    Q_OBJECT

private slots:
    void emptyApplyClearsFilterInsteadOfRequestingEmptyPattern();
    void nonEmptyApplyRequestsFilterAndAddsHistory();
    void applyTrimsPatternBeforeRequestAndHistory();
    void invalidRegexDoesNotRequestFilterOrAddHistory();
    void repeatedApplyMovesHistoryItemToTopWithoutDuplicate();
};

void TerminalFilterBarTest::emptyApplyClearsFilterInsteadOfRequestingEmptyPattern()
{
    TerminalFilterBar bar;
    auto* applyButton = bar.findChild<QPushButton*>("filterApplyBtn");
    QVERIFY(applyButton);
    QSignalSpy requestedSpy(&bar, &TerminalFilterBar::filterRequested);
    QSignalSpy clearedSpy(&bar, &TerminalFilterBar::filterCleared);

    QTest::mouseClick(applyButton, Qt::LeftButton);

    QCOMPARE(requestedSpy.count(), 0);
    QCOMPARE(clearedSpy.count(), 1);
}

void TerminalFilterBarTest::nonEmptyApplyRequestsFilterAndAddsHistory()
{
    TerminalFilterBar bar;
    auto* input = bar.findChild<QLineEdit*>("filterPatternEdit");
    auto* history = bar.findChild<QComboBox*>("filterHistoryCombo");
    auto* applyButton = bar.findChild<QPushButton*>("filterApplyBtn");
    QVERIFY(input);
    QVERIFY(history);
    QVERIFY(applyButton);
    QSignalSpy requestedSpy(&bar, &TerminalFilterBar::filterRequested);
    QSignalSpy clearedSpy(&bar, &TerminalFilterBar::filterCleared);

    input->setText(QStringLiteral("ERROR"));
    QTest::mouseClick(applyButton, Qt::LeftButton);

    QCOMPARE(requestedSpy.count(), 1);
    QCOMPARE(clearedSpy.count(), 0);
    QCOMPARE(requestedSpy.first().at(0).toString(), QStringLiteral("ERROR"));
    QCOMPARE(history->itemText(0), QStringLiteral("ERROR"));
}

void TerminalFilterBarTest::applyTrimsPatternBeforeRequestAndHistory()
{
    TerminalFilterBar bar;
    auto* input = bar.findChild<QLineEdit*>("filterPatternEdit");
    auto* history = bar.findChild<QComboBox*>("filterHistoryCombo");
    auto* applyButton = bar.findChild<QPushButton*>("filterApplyBtn");
    QVERIFY(input);
    QVERIFY(history);
    QVERIFY(applyButton);
    QSignalSpy requestedSpy(&bar, &TerminalFilterBar::filterRequested);

    input->setText(QStringLiteral("  ERROR  "));
    QTest::mouseClick(applyButton, Qt::LeftButton);

    QCOMPARE(requestedSpy.count(), 1);
    QCOMPARE(requestedSpy.first().at(0).toString(), QStringLiteral("ERROR"));
    QCOMPARE(input->text(), QStringLiteral("ERROR"));
    QCOMPARE(history->itemText(0), QStringLiteral("ERROR"));
}

void TerminalFilterBarTest::invalidRegexDoesNotRequestFilterOrAddHistory()
{
    TerminalFilterBar bar;
    auto* input = bar.findChild<QLineEdit*>("filterPatternEdit");
    auto* history = bar.findChild<QComboBox*>("filterHistoryCombo");
    auto* applyButton = bar.findChild<QPushButton*>("filterApplyBtn");
    QVERIFY(input);
    QVERIFY(history);
    QVERIFY(applyButton);
    QSignalSpy requestedSpy(&bar, &TerminalFilterBar::filterRequested);
    QSignalSpy clearedSpy(&bar, &TerminalFilterBar::filterCleared);

    input->setText(QStringLiteral("["));
    QTest::mouseClick(applyButton, Qt::LeftButton);

    QCOMPARE(requestedSpy.count(), 0);
    QCOMPARE(clearedSpy.count(), 0);
    QCOMPARE(history->count(), 0);
    QCOMPARE(bar.totalFilterChanges(), quint64(0));
    QVERIFY(input->toolTip().startsWith(QStringLiteral("正则表达式无效")));
}

void TerminalFilterBarTest::repeatedApplyMovesHistoryItemToTopWithoutDuplicate()
{
    TerminalFilterBar bar;
    auto* input = bar.findChild<QLineEdit*>("filterPatternEdit");
    auto* history = bar.findChild<QComboBox*>("filterHistoryCombo");
    auto* applyButton = bar.findChild<QPushButton*>("filterApplyBtn");
    QVERIFY(input);
    QVERIFY(history);
    QVERIFY(applyButton);

    input->setText(QStringLiteral("WARN"));
    QTest::mouseClick(applyButton, Qt::LeftButton);
    input->setText(QStringLiteral("ERROR"));
    QTest::mouseClick(applyButton, Qt::LeftButton);
    input->setText(QStringLiteral("WARN"));
    QTest::mouseClick(applyButton, Qt::LeftButton);

    QCOMPARE(history->count(), 2);
    QCOMPARE(history->itemText(0), QStringLiteral("WARN"));
    QCOMPARE(history->itemText(1), QStringLiteral("ERROR"));
}

QTEST_MAIN(TerminalFilterBarTest)
#include "test_terminal_filter_bar.moc"
