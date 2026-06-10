#include <QtTest/QtTest>

#include "serial/commands/QuickCommandBar.h"

#include <QPushButton>
#include <QSignalSpy>

class QuickCommandBarTest : public QObject {
    Q_OBJECT

private slots:
    void emptyCommandButtonIsDisabled();
    void invalidHexCommandButtonIsDisabled();
    void blankCommandNameUsesReadableFallbackLabel();
    void textCommandEmitsPayload();
};

void QuickCommandBarTest::emptyCommandButtonIsDisabled()
{
    QuickCommandBar bar;
    bar.setCommands({{QStringLiteral("指令"), QString(), false}});

    auto* button = bar.findChild<QPushButton*>("quickCmdBtn_0");
    QVERIFY(button);
    QVERIFY(!button->isEnabled());
    QVERIFY(button->toolTip().contains(QStringLiteral("编辑")));
}

void QuickCommandBarTest::invalidHexCommandButtonIsDisabled()
{
    QuickCommandBar bar;
    bar.setCommands({{QStringLiteral("Bad"), QStringLiteral("AA 5"), true}});
    QSignalSpy triggeredSpy(&bar, &QuickCommandBar::commandTriggered);
    QSignalSpy errorSpy(&bar, &QuickCommandBar::commandError);

    auto* button = bar.findChild<QPushButton*>("quickCmdBtn_0");
    QVERIFY(button);
    QVERIFY(!button->isEnabled());
    QVERIFY(button->toolTip().contains(QStringLiteral("HEX")));

    QTest::mouseClick(button, Qt::LeftButton);
    QCOMPARE(triggeredSpy.count(), 0);
    QCOMPARE(errorSpy.count(), 0);
}

void QuickCommandBarTest::blankCommandNameUsesReadableFallbackLabel()
{
    QuickCommandBar bar;
    bar.setCommands({{QStringLiteral("   "), QStringLiteral("AT"), false}});

    auto* button = bar.findChild<QPushButton*>("quickCmdBtn_0");
    QVERIFY(button);
    QVERIFY(!button->text().trimmed().isEmpty());
    QVERIFY(button->text().contains(QStringLiteral("指令")));
}

void QuickCommandBarTest::textCommandEmitsPayload()
{
    QuickCommandBar bar;
    bar.setCommands({{QStringLiteral("AT"), QStringLiteral("AT\r\n"), false}});
    QSignalSpy spy(&bar, &QuickCommandBar::commandTriggered);

    auto* button = bar.findChild<QPushButton*>("quickCmdBtn_0");
    QVERIFY(button);
    QTest::mouseClick(button, Qt::LeftButton);

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.first().first().toByteArray(), QByteArray("AT\r\n"));
}

QTEST_MAIN(QuickCommandBarTest)
#include "test_quick_command_bar.moc"
