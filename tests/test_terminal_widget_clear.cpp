#include <QtTest/QtTest>

#include "terminal/model/TerminalModel.h"
#include "terminal/widget/TerminalWidget.h"

class TerminalWidgetClearTest : public QObject {
    Q_OBJECT

private slots:
    void clearResetsSearchState();
};

void TerminalWidgetClearTest::clearResetsSearchState()
{
    TerminalModel model;
    TerminalWidget terminal;
    terminal.setModel(&model);

    model.appendReceived("hello terminal");
    terminal.resize(640, 240);
    terminal.show();
    QVERIFY(QTest::qWaitForWindowExposed(&terminal));
    terminal.repaint();
    terminal.setSearchHighlight(QStringLiteral("terminal"), false, false);

    QCOMPARE(terminal.searchMatchCount(), 1);

    terminal.clear();

    QCOMPARE(terminal.searchMatchCount(), 0);
    QCOMPARE(terminal.currentMatchIndex(), -1);
    QVERIFY(terminal.searchManager()->searchPattern().isEmpty());
}

QTEST_MAIN(TerminalWidgetClearTest)
#include "test_terminal_widget_clear.moc"
