#include <QtTest/QtTest>

#include "core/mainwindow/StartupOptions.h"

class StartupOptionsTest : public QObject {
    Q_OBJECT

private slots:
    void keepsEmptyPanelWhenNoStartupTarget();
    void parsesPanelFlagValue();
    void parsesPanelEqualsValue();
    void parsesSerialStationAliasValue();
    void parsesSerialStationAliasEqualsValue();
    void ignoresUnknownStationAlias();
    void ignoresMissingPanelValue();
};

void StartupOptionsTest::keepsEmptyPanelWhenNoStartupTarget()
{
    const StartupOptions options = StartupOptions::fromArguments({
        QStringLiteral("EmbedDebug.exe"),
    });

    QCOMPARE(options.panelId(), QString());
}

void StartupOptionsTest::parsesPanelFlagValue()
{
    const StartupOptions options = StartupOptions::fromArguments({
        QStringLiteral("EmbedDebug.exe"),
        QStringLiteral("--panel"),
        QStringLiteral("serial.station"),
    });

    QCOMPARE(options.panelId(), QStringLiteral("serial.station"));
}

void StartupOptionsTest::parsesPanelEqualsValue()
{
    const StartupOptions options = StartupOptions::fromArguments({
        QStringLiteral("EmbedDebug.exe"),
        QStringLiteral("--panel=serial.station"),
    });

    QCOMPARE(options.panelId(), QStringLiteral("serial.station"));
}

void StartupOptionsTest::parsesSerialStationAliasValue()
{
    const StartupOptions options = StartupOptions::fromArguments({
        QStringLiteral("EmbedDebug.exe"),
        QStringLiteral("--station"),
        QStringLiteral("serial"),
    });

    QCOMPARE(options.panelId(), QStringLiteral("serial.station"));
}

void StartupOptionsTest::parsesSerialStationAliasEqualsValue()
{
    const StartupOptions options = StartupOptions::fromArguments({
        QStringLiteral("EmbedDebug.exe"),
        QStringLiteral("--station=serial-station"),
    });

    QCOMPARE(options.panelId(), QStringLiteral("serial.station"));
}

void StartupOptionsTest::ignoresUnknownStationAlias()
{
    const StartupOptions options = StartupOptions::fromArguments({
        QStringLiteral("EmbedDebug.exe"),
        QStringLiteral("--station"),
        QStringLiteral("can"),
    });

    QCOMPARE(options.panelId(), QString());
}

void StartupOptionsTest::ignoresMissingPanelValue()
{
    const StartupOptions options = StartupOptions::fromArguments({
        QStringLiteral("EmbedDebug.exe"),
        QStringLiteral("--panel"),
        QStringLiteral("--station"),
        QStringLiteral("serial"),
    });

    QCOMPARE(options.panelId(), QString());
}

QTEST_MAIN(StartupOptionsTest)
#include "test_startup_options.moc"
