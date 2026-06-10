#include <QtTest/QtTest>

#include "core/mainwindow/MainWindowLayoutState.h"

class MainWindowLayoutStateTest : public QObject {
    Q_OBJECT

private slots:
    void savesTreeWidthWithoutIconNavBar();
    void savesTreeWidthWithIconNavBar();
    void restoresSplitterSizesWithoutIconNavBar();
    void restoresSplitterSizesWithIconNavBar();
    void clampsRestoredSizesToAvailableContent();
    void detectsCollapsedTreeWithIconNavBar();
};

void MainWindowLayoutStateTest::savesTreeWidthWithoutIconNavBar()
{
    QCOMPARE(savedNavTreeWidthFromSplitterSizes({240, 960}, false), 240);
}

void MainWindowLayoutStateTest::savesTreeWidthWithIconNavBar()
{
    QCOMPARE(savedNavTreeWidthFromSplitterSizes({56, 260, 884}, true), 260);
}

void MainWindowLayoutStateTest::restoresSplitterSizesWithoutIconNavBar()
{
    QCOMPARE(restoredNavigationSplitterSizes(240, 1200, false), QList<int>({240, 960}));
}

void MainWindowLayoutStateTest::restoresSplitterSizesWithIconNavBar()
{
    QCOMPARE(restoredNavigationSplitterSizes(260, 1200, true), QList<int>({56, 260, 884}));
}

void MainWindowLayoutStateTest::clampsRestoredSizesToAvailableContent()
{
    QCOMPARE(restoredNavigationSplitterSizes(2000, 900, true), QList<int>({56, 843, 1}));
    QCOMPARE(restoredNavigationSplitterSizes(2000, 900, false), QList<int>({899, 1}));
}

void MainWindowLayoutStateTest::detectsCollapsedTreeWithIconNavBar()
{
    QVERIFY(navTreeIsCollapsedInSplitterSizes({56, 0, 1144}, true));
    QVERIFY(!navTreeIsCollapsedInSplitterSizes({56, 220, 924}, true));
    QVERIFY(navTreeIsCollapsedInSplitterSizes({0, 1200}, false));
}

QTEST_GUILESS_MAIN(MainWindowLayoutStateTest)
#include "test_mainwindow_layout_state.moc"
