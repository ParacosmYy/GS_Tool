#include <QtTest/QtTest>

#include <QLineEdit>
#include <QListWidget>
#include <QSignalSpy>

#include "core/widgets/CommandPalette.h"

class CommandPaletteTest : public QObject {
    Q_OBJECT

private slots:
    void enterInSearchExecutesCurrentCommand();
    void searchMatchesCommandId();
    void prefixMatchesRankBeforeSubsequenceMatches();
    void downThenEnterExecutesSecondCommandFromSearch();
    void noResultsShowsDisabledEmptyItem();
};

void CommandPaletteTest::enterInSearchExecutesCurrentCommand()
{
    CommandPalette palette;
    bool executed = false;
    palette.registerCommand({
        "nav.serial",
        QStringLiteral("导航"),
        QStringLiteral("串口配置"),
        QString(),
        [&executed]() { executed = true; },
    });
    QSignalSpy spy(&palette, &CommandPalette::commandExecuted);

    palette.showPalette();
    auto* searchEdit = palette.findChild<QLineEdit*>("commandPaletteSearch");
    QVERIFY(searchEdit);

    QTest::keyClick(searchEdit, Qt::Key_Return);

    QVERIFY(executed);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.first().first().toString(), QStringLiteral("nav.serial"));
}

void CommandPaletteTest::searchMatchesCommandId()
{
    CommandPalette palette;
    palette.registerCommand({
        "nav.serial.config",
        QStringLiteral("导航"),
        QStringLiteral("串口配置"),
        QString(),
        []() {},
    });
    palette.registerCommand({
        "nav.chart.main",
        QStringLiteral("导航"),
        QStringLiteral("波形图"),
        QString(),
        []() {},
    });

    palette.showPalette();
    auto* searchEdit = palette.findChild<QLineEdit*>("commandPaletteSearch");
    auto* listWidget = palette.findChild<QListWidget*>("commandPaletteList");
    QVERIFY(searchEdit);
    QVERIFY(listWidget);

    QTest::keyClicks(searchEdit, "serial");

    QCOMPARE(listWidget->count(), 1);
    QCOMPARE(listWidget->currentItem()->data(Qt::UserRole).toInt(), 0);
}

void CommandPaletteTest::prefixMatchesRankBeforeSubsequenceMatches()
{
    CommandPalette palette;
    palette.registerCommand({
        "nav.transfer",
        QStringLiteral("导航"),
        QStringLiteral("数据发送"),
        QString(),
        []() {},
    });
    palette.registerCommand({
        "nav.terminal",
        QStringLiteral("导航"),
        QStringLiteral("终端"),
        QString(),
        []() {},
    });

    palette.showPalette();
    auto* searchEdit = palette.findChild<QLineEdit*>("commandPaletteSearch");
    auto* listWidget = palette.findChild<QListWidget*>("commandPaletteList");
    QVERIFY(searchEdit);
    QVERIFY(listWidget);

    QTest::keyClicks(searchEdit, "te");

    QCOMPARE(listWidget->count(), 2);
    QCOMPARE(listWidget->item(0)->data(Qt::UserRole).toInt(), 1);
}

void CommandPaletteTest::downThenEnterExecutesSecondCommandFromSearch()
{
    CommandPalette palette;
    bool firstExecuted = false;
    bool secondExecuted = false;
    palette.registerCommand({
        "nav.serial",
        QStringLiteral("导航"),
        QStringLiteral("串口配置"),
        QString(),
        [&firstExecuted]() { firstExecuted = true; },
    });
    palette.registerCommand({
        "nav.chart",
        QStringLiteral("导航"),
        QStringLiteral("波形图"),
        QString(),
        [&secondExecuted]() { secondExecuted = true; },
    });

    palette.showPalette();
    auto* searchEdit = palette.findChild<QLineEdit*>("commandPaletteSearch");
    QVERIFY(searchEdit);

    QTest::keyClick(searchEdit, Qt::Key_Down);
    QTest::keyClick(searchEdit, Qt::Key_Return);

    QVERIFY(!firstExecuted);
    QVERIFY(secondExecuted);
}

void CommandPaletteTest::noResultsShowsDisabledEmptyItem()
{
    CommandPalette palette;
    bool executed = false;
    palette.registerCommand({
        "nav.serial",
        QStringLiteral("导航"),
        QStringLiteral("串口配置"),
        QString(),
        [&executed]() { executed = true; },
    });

    palette.showPalette();
    auto* searchEdit = palette.findChild<QLineEdit*>("commandPaletteSearch");
    auto* listWidget = palette.findChild<QListWidget*>("commandPaletteList");
    QVERIFY(searchEdit);
    QVERIFY(listWidget);

    QTest::keyClicks(searchEdit, "zzzz");

    QCOMPARE(listWidget->count(), 1);
    QVERIFY(!(listWidget->item(0)->flags() & Qt::ItemIsEnabled));
    QCOMPARE(listWidget->item(0)->text(), QStringLiteral("未找到匹配项"));

    QTest::keyClick(searchEdit, Qt::Key_Return);
    QVERIFY(!executed);
}

QTEST_MAIN(CommandPaletteTest)
#include "test_command_palette.moc"
