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
    void parsesProfileFlagAndRoutesToSerialStation();
    void parsesProfileEqualsAndRoutesToSerialStation();
    void parsesSerialProfileFlagAndRoutesToSerialStation();
    void parsesSerialProfileEqualsAndRoutesToSerialStation();
    void ignoresMissingProfileValue();
    void keepsProfilePathWithSpaces();
    void preservesExplicitPanelWhenProfileIsProvided();
    void lastPanelLikeArgumentWinsBeforeProfileDefaulting();
    void lastProfileLikeArgumentWins();
    void ignoresEmptyProfileEquals();
    void ignoresEmptySerialProfileEquals();
    void rejectsMissingSerialProfileValue();
    void rejectsMissingProfileValueAtEnd();
    void routesSerialAliasBeforeProfile();
    void routesSerialAliasAfterProfile();
    void keepsUnknownStationEmptyWithoutProfile();
    void unknownStationWithProfileFallsBackToSerialStation();
    void trimsPanelFlagValue();
    void trimsPanelEqualsValue();
    void trimsStationAliasValue();
    void trimsProfileFlagValue();
    void trimsProfileEqualsValue();
    void ignoresUnknownArgumentsBeforeProfile();
    void ignoresUnknownArgumentsAfterProfile();
    void unknownArgumentBetweenFlagAndValueInvalidatesFlag();
    void keepsPanelWhenEmptyProfileEqualsIsIgnored();
    void keepsProfileWhenEmptyPanelEqualsIsIgnored();
    void parsesLastProfileFlagAndRoutesToSerialStation();
    void parsesSerialLastProfileFlagAndRoutesToSerialStation();
    void parsesLastProfileEqualsTrueValues_data();
    void parsesLastProfileEqualsTrueValues();
    void parsesLastProfileEqualsFalseValues_data();
    void parsesLastProfileEqualsFalseValues();
    void parsesSerialLastProfileEqualsTrue();
    void parsesSerialLastProfileEqualsFalse();
    void explicitProfileWinsOverLastProfileFlag();
    void lastProfileDoesNotReplaceExplicitPanel();
    void lastProfileWithSerialStationAliasKeepsSerialPanel();
    void lastProfileFlagBeforeProfileKeepsExplicitProfilePath();
    void lastProfileFlagAfterProfileKeepsExplicitProfilePath();
    void lastProfileFalseDoesNotRouteToSerialStation();
    void serialLastProfileFalseDoesNotRouteToSerialStation();
    void repeatedLastProfileLikeArgumentsUseLastBooleanValue();
    void unknownLastProfileValueIsIgnored();
    void lastProfileEqualsTrimsValue();
    void lastProfileFlagDoesNotConsumeNextArgument();
    void lastProfileFlagCanAppearBetweenProfileFlagAndValue();
    void lastProfileEqualsTrueKeepsExplicitPanel();
    void serialLastProfileFlagDoesNotConsumeStationFlag();
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

void StartupOptionsTest::parsesProfileFlagAndRoutesToSerialStation()
{
    const StartupOptions options = StartupOptions::fromArguments({
        QStringLiteral("EmbedDebug.exe"),
        QStringLiteral("--profile"),
        QStringLiteral("profiles/line-a.edserialprofile"),
    });

    QCOMPARE(options.panelId(), QStringLiteral("serial.station"));
    QCOMPARE(options.profileFilePath(), QStringLiteral("profiles/line-a.edserialprofile"));
}

void StartupOptionsTest::parsesProfileEqualsAndRoutesToSerialStation()
{
    const StartupOptions options = StartupOptions::fromArguments({
        QStringLiteral("EmbedDebug.exe"),
        QStringLiteral("--profile=C:/profiles/line-a.edserialprofile"),
    });

    QCOMPARE(options.panelId(), QStringLiteral("serial.station"));
    QCOMPARE(options.profileFilePath(), QStringLiteral("C:/profiles/line-a.edserialprofile"));
}

void StartupOptionsTest::parsesSerialProfileFlagAndRoutesToSerialStation()
{
    const StartupOptions options = StartupOptions::fromArguments({
        QStringLiteral("EmbedDebug.exe"),
        QStringLiteral("--serial-profile"),
        QStringLiteral("line-a.edserialprofile"),
    });

    QCOMPARE(options.panelId(), QStringLiteral("serial.station"));
    QCOMPARE(options.profileFilePath(), QStringLiteral("line-a.edserialprofile"));
}

void StartupOptionsTest::parsesSerialProfileEqualsAndRoutesToSerialStation()
{
    const StartupOptions options = StartupOptions::fromArguments({
        QStringLiteral("EmbedDebug.exe"),
        QStringLiteral("--serial-profile=line-a.edserialprofile"),
    });

    QCOMPARE(options.panelId(), QStringLiteral("serial.station"));
    QCOMPARE(options.profileFilePath(), QStringLiteral("line-a.edserialprofile"));
}

void StartupOptionsTest::ignoresMissingProfileValue()
{
    const StartupOptions options = StartupOptions::fromArguments({
        QStringLiteral("EmbedDebug.exe"),
        QStringLiteral("--profile"),
        QStringLiteral("--station"),
        QStringLiteral("serial"),
    });

    QCOMPARE(options.panelId(), QString());
    QCOMPARE(options.profileFilePath(), QString());
}

void StartupOptionsTest::keepsProfilePathWithSpaces()
{
    const StartupOptions options = StartupOptions::fromArguments({
        QStringLiteral("EmbedDebug.exe"),
        QStringLiteral("--profile"),
        QStringLiteral("C:/Factory Profiles/line a.edserialprofile"),
    });

    QCOMPARE(options.panelId(), QStringLiteral("serial.station"));
    QCOMPARE(options.profileFilePath(), QStringLiteral("C:/Factory Profiles/line a.edserialprofile"));
}

void StartupOptionsTest::preservesExplicitPanelWhenProfileIsProvided()
{
    const StartupOptions options = StartupOptions::fromArguments({
        QStringLiteral("EmbedDebug.exe"),
        QStringLiteral("--panel"),
        QStringLiteral("diagnostics.dashboard"),
        QStringLiteral("--profile"),
        QStringLiteral("profiles/line-a.edserialprofile"),
    });

    QCOMPARE(options.panelId(), QStringLiteral("diagnostics.dashboard"));
    QCOMPARE(options.profileFilePath(), QStringLiteral("profiles/line-a.edserialprofile"));
}

void StartupOptionsTest::lastPanelLikeArgumentWinsBeforeProfileDefaulting()
{
    const StartupOptions options = StartupOptions::fromArguments({
        QStringLiteral("EmbedDebug.exe"),
        QStringLiteral("--station"),
        QStringLiteral("serial"),
        QStringLiteral("--panel"),
        QStringLiteral("diagnostics.dashboard"),
        QStringLiteral("--profile"),
        QStringLiteral("profiles/line-a.edserialprofile"),
    });

    QCOMPARE(options.panelId(), QStringLiteral("diagnostics.dashboard"));
    QCOMPARE(options.profileFilePath(), QStringLiteral("profiles/line-a.edserialprofile"));
}

void StartupOptionsTest::lastProfileLikeArgumentWins()
{
    const StartupOptions options = StartupOptions::fromArguments({
        QStringLiteral("EmbedDebug.exe"),
        QStringLiteral("--profile"),
        QStringLiteral("profiles/line-a.edserialprofile"),
        QStringLiteral("--serial-profile=profiles/line-b.edserialprofile"),
    });

    QCOMPARE(options.panelId(), QStringLiteral("serial.station"));
    QCOMPARE(options.profileFilePath(), QStringLiteral("profiles/line-b.edserialprofile"));
}

void StartupOptionsTest::ignoresEmptyProfileEquals()
{
    const StartupOptions options = StartupOptions::fromArguments({
        QStringLiteral("EmbedDebug.exe"),
        QStringLiteral("--profile="),
    });

    QCOMPARE(options.panelId(), QString());
    QCOMPARE(options.profileFilePath(), QString());
}

void StartupOptionsTest::ignoresEmptySerialProfileEquals()
{
    const StartupOptions options = StartupOptions::fromArguments({
        QStringLiteral("EmbedDebug.exe"),
        QStringLiteral("--serial-profile="),
    });

    QCOMPARE(options.panelId(), QString());
    QCOMPARE(options.profileFilePath(), QString());
}

void StartupOptionsTest::rejectsMissingSerialProfileValue()
{
    const StartupOptions options = StartupOptions::fromArguments({
        QStringLiteral("EmbedDebug.exe"),
        QStringLiteral("--serial-profile"),
        QStringLiteral("--panel"),
        QStringLiteral("diagnostics.dashboard"),
    });

    QCOMPARE(options.panelId(), QString());
    QCOMPARE(options.profileFilePath(), QString());
}

void StartupOptionsTest::rejectsMissingProfileValueAtEnd()
{
    const StartupOptions options = StartupOptions::fromArguments({
        QStringLiteral("EmbedDebug.exe"),
        QStringLiteral("--station"),
        QStringLiteral("serial"),
        QStringLiteral("--profile"),
    });

    QCOMPARE(options.panelId(), QString());
    QCOMPARE(options.profileFilePath(), QString());
}

void StartupOptionsTest::routesSerialAliasBeforeProfile()
{
    const StartupOptions options = StartupOptions::fromArguments({
        QStringLiteral("EmbedDebug.exe"),
        QStringLiteral("--station"),
        QStringLiteral("serial_station"),
        QStringLiteral("--profile"),
        QStringLiteral("profiles/line-a.edserialprofile"),
    });

    QCOMPARE(options.panelId(), QStringLiteral("serial.station"));
    QCOMPARE(options.profileFilePath(), QStringLiteral("profiles/line-a.edserialprofile"));
}

void StartupOptionsTest::routesSerialAliasAfterProfile()
{
    const StartupOptions options = StartupOptions::fromArguments({
        QStringLiteral("EmbedDebug.exe"),
        QStringLiteral("--profile"),
        QStringLiteral("profiles/line-a.edserialprofile"),
        QStringLiteral("--station=serial-station"),
    });

    QCOMPARE(options.panelId(), QStringLiteral("serial.station"));
    QCOMPARE(options.profileFilePath(), QStringLiteral("profiles/line-a.edserialprofile"));
}

void StartupOptionsTest::keepsUnknownStationEmptyWithoutProfile()
{
    const StartupOptions options = StartupOptions::fromArguments({
        QStringLiteral("EmbedDebug.exe"),
        QStringLiteral("--station=unknown"),
    });

    QCOMPARE(options.panelId(), QString());
    QCOMPARE(options.profileFilePath(), QString());
}

void StartupOptionsTest::unknownStationWithProfileFallsBackToSerialStation()
{
    const StartupOptions options = StartupOptions::fromArguments({
        QStringLiteral("EmbedDebug.exe"),
        QStringLiteral("--station=unknown"),
        QStringLiteral("--profile=profiles/line-a.edserialprofile"),
    });

    QCOMPARE(options.panelId(), QStringLiteral("serial.station"));
    QCOMPARE(options.profileFilePath(), QStringLiteral("profiles/line-a.edserialprofile"));
}

void StartupOptionsTest::trimsPanelFlagValue()
{
    const StartupOptions options = StartupOptions::fromArguments({
        QStringLiteral("EmbedDebug.exe"),
        QStringLiteral("--panel"),
        QStringLiteral("  serial.station  "),
    });

    QCOMPARE(options.panelId(), QStringLiteral("serial.station"));
    QCOMPARE(options.profileFilePath(), QString());
}

void StartupOptionsTest::trimsPanelEqualsValue()
{
    const StartupOptions options = StartupOptions::fromArguments({
        QStringLiteral("EmbedDebug.exe"),
        QStringLiteral("--panel=  serial.station  "),
    });

    QCOMPARE(options.panelId(), QStringLiteral("serial.station"));
    QCOMPARE(options.profileFilePath(), QString());
}

void StartupOptionsTest::trimsStationAliasValue()
{
    const StartupOptions options = StartupOptions::fromArguments({
        QStringLiteral("EmbedDebug.exe"),
        QStringLiteral("--station"),
        QStringLiteral("  SERIAL_STATION  "),
    });

    QCOMPARE(options.panelId(), QStringLiteral("serial.station"));
    QCOMPARE(options.profileFilePath(), QString());
}

void StartupOptionsTest::trimsProfileFlagValue()
{
    const StartupOptions options = StartupOptions::fromArguments({
        QStringLiteral("EmbedDebug.exe"),
        QStringLiteral("--profile"),
        QStringLiteral("  profiles/line-a.edserialprofile  "),
    });

    QCOMPARE(options.panelId(), QStringLiteral("serial.station"));
    QCOMPARE(options.profileFilePath(), QStringLiteral("profiles/line-a.edserialprofile"));
}

void StartupOptionsTest::trimsProfileEqualsValue()
{
    const StartupOptions options = StartupOptions::fromArguments({
        QStringLiteral("EmbedDebug.exe"),
        QStringLiteral("--serial-profile=  profiles/line-a.edserialprofile  "),
    });

    QCOMPARE(options.panelId(), QStringLiteral("serial.station"));
    QCOMPARE(options.profileFilePath(), QStringLiteral("profiles/line-a.edserialprofile"));
}

void StartupOptionsTest::ignoresUnknownArgumentsBeforeProfile()
{
    const StartupOptions options = StartupOptions::fromArguments({
        QStringLiteral("EmbedDebug.exe"),
        QStringLiteral("--verbose"),
        QStringLiteral("--profile"),
        QStringLiteral("profiles/line-a.edserialprofile"),
    });

    QCOMPARE(options.panelId(), QStringLiteral("serial.station"));
    QCOMPARE(options.profileFilePath(), QStringLiteral("profiles/line-a.edserialprofile"));
}

void StartupOptionsTest::ignoresUnknownArgumentsAfterProfile()
{
    const StartupOptions options = StartupOptions::fromArguments({
        QStringLiteral("EmbedDebug.exe"),
        QStringLiteral("--profile"),
        QStringLiteral("profiles/line-a.edserialprofile"),
        QStringLiteral("--verbose"),
    });

    QCOMPARE(options.panelId(), QStringLiteral("serial.station"));
    QCOMPARE(options.profileFilePath(), QStringLiteral("profiles/line-a.edserialprofile"));
}

void StartupOptionsTest::unknownArgumentBetweenFlagAndValueInvalidatesFlag()
{
    const StartupOptions options = StartupOptions::fromArguments({
        QStringLiteral("EmbedDebug.exe"),
        QStringLiteral("--profile"),
        QStringLiteral("--verbose"),
        QStringLiteral("profiles/line-a.edserialprofile"),
    });

    QCOMPARE(options.panelId(), QString());
    QCOMPARE(options.profileFilePath(), QString());
}

void StartupOptionsTest::keepsPanelWhenEmptyProfileEqualsIsIgnored()
{
    const StartupOptions options = StartupOptions::fromArguments({
        QStringLiteral("EmbedDebug.exe"),
        QStringLiteral("--panel"),
        QStringLiteral("diagnostics.dashboard"),
        QStringLiteral("--profile="),
    });

    QCOMPARE(options.panelId(), QStringLiteral("diagnostics.dashboard"));
    QCOMPARE(options.profileFilePath(), QString());
}

void StartupOptionsTest::keepsProfileWhenEmptyPanelEqualsIsIgnored()
{
    const StartupOptions options = StartupOptions::fromArguments({
        QStringLiteral("EmbedDebug.exe"),
        QStringLiteral("--profile=profiles/line-a.edserialprofile"),
        QStringLiteral("--panel="),
    });

    QCOMPARE(options.panelId(), QStringLiteral("serial.station"));
    QCOMPARE(options.profileFilePath(), QStringLiteral("profiles/line-a.edserialprofile"));
}

void StartupOptionsTest::parsesLastProfileFlagAndRoutesToSerialStation()
{
    const StartupOptions options = StartupOptions::fromArguments({
        QStringLiteral("EmbedDebug.exe"),
        QStringLiteral("--last-profile"),
    });

    QCOMPARE(options.panelId(), QStringLiteral("serial.station"));
    QVERIFY(options.loadLastProfile());
    QCOMPARE(options.profileFilePath(), QString());
}

void StartupOptionsTest::parsesSerialLastProfileFlagAndRoutesToSerialStation()
{
    const StartupOptions options = StartupOptions::fromArguments({
        QStringLiteral("EmbedDebug.exe"),
        QStringLiteral("--serial-last-profile"),
    });

    QCOMPARE(options.panelId(), QStringLiteral("serial.station"));
    QVERIFY(options.loadLastProfile());
    QCOMPARE(options.profileFilePath(), QString());
}

void StartupOptionsTest::parsesLastProfileEqualsTrueValues_data()
{
    QTest::addColumn<QString>("flag");

    QTest::newRow("true") << QStringLiteral("--last-profile=true");
    QTest::newRow("one") << QStringLiteral("--last-profile=1");
    QTest::newRow("yes") << QStringLiteral("--last-profile=yes");
    QTest::newRow("on") << QStringLiteral("--last-profile=on");
    QTest::newRow("mixed-case") << QStringLiteral("--last-profile=TrUe");
}

void StartupOptionsTest::parsesLastProfileEqualsTrueValues()
{
    QFETCH(QString, flag);

    const StartupOptions options = StartupOptions::fromArguments({
        QStringLiteral("EmbedDebug.exe"),
        flag,
    });

    QCOMPARE(options.panelId(), QStringLiteral("serial.station"));
    QVERIFY(options.loadLastProfile());
}

void StartupOptionsTest::parsesLastProfileEqualsFalseValues_data()
{
    QTest::addColumn<QString>("flag");

    QTest::newRow("false") << QStringLiteral("--last-profile=false");
    QTest::newRow("zero") << QStringLiteral("--last-profile=0");
    QTest::newRow("no") << QStringLiteral("--last-profile=no");
    QTest::newRow("off") << QStringLiteral("--last-profile=off");
    QTest::newRow("empty") << QStringLiteral("--last-profile=");
}

void StartupOptionsTest::parsesLastProfileEqualsFalseValues()
{
    QFETCH(QString, flag);

    const StartupOptions options = StartupOptions::fromArguments({
        QStringLiteral("EmbedDebug.exe"),
        flag,
    });

    QCOMPARE(options.panelId(), QString());
    QVERIFY(!options.loadLastProfile());
}

void StartupOptionsTest::parsesSerialLastProfileEqualsTrue()
{
    const StartupOptions options = StartupOptions::fromArguments({
        QStringLiteral("EmbedDebug.exe"),
        QStringLiteral("--serial-last-profile=true"),
    });

    QCOMPARE(options.panelId(), QStringLiteral("serial.station"));
    QVERIFY(options.loadLastProfile());
}

void StartupOptionsTest::parsesSerialLastProfileEqualsFalse()
{
    const StartupOptions options = StartupOptions::fromArguments({
        QStringLiteral("EmbedDebug.exe"),
        QStringLiteral("--serial-last-profile=false"),
    });

    QCOMPARE(options.panelId(), QString());
    QVERIFY(!options.loadLastProfile());
}

void StartupOptionsTest::explicitProfileWinsOverLastProfileFlag()
{
    const StartupOptions options = StartupOptions::fromArguments({
        QStringLiteral("EmbedDebug.exe"),
        QStringLiteral("--last-profile"),
        QStringLiteral("--profile"),
        QStringLiteral("profiles/line-a.edserialprofile"),
    });

    QCOMPARE(options.panelId(), QStringLiteral("serial.station"));
    QVERIFY(options.loadLastProfile());
    QCOMPARE(options.profileFilePath(), QStringLiteral("profiles/line-a.edserialprofile"));
}

void StartupOptionsTest::lastProfileDoesNotReplaceExplicitPanel()
{
    const StartupOptions options = StartupOptions::fromArguments({
        QStringLiteral("EmbedDebug.exe"),
        QStringLiteral("--panel"),
        QStringLiteral("diagnostics.dashboard"),
        QStringLiteral("--last-profile"),
    });

    QCOMPARE(options.panelId(), QStringLiteral("diagnostics.dashboard"));
    QVERIFY(options.loadLastProfile());
}

void StartupOptionsTest::lastProfileWithSerialStationAliasKeepsSerialPanel()
{
    const StartupOptions options = StartupOptions::fromArguments({
        QStringLiteral("EmbedDebug.exe"),
        QStringLiteral("--station=serial"),
        QStringLiteral("--last-profile"),
    });

    QCOMPARE(options.panelId(), QStringLiteral("serial.station"));
    QVERIFY(options.loadLastProfile());
}

void StartupOptionsTest::lastProfileFlagBeforeProfileKeepsExplicitProfilePath()
{
    const StartupOptions options = StartupOptions::fromArguments({
        QStringLiteral("EmbedDebug.exe"),
        QStringLiteral("--last-profile"),
        QStringLiteral("--serial-profile=profiles/line-b.edserialprofile"),
    });

    QCOMPARE(options.panelId(), QStringLiteral("serial.station"));
    QVERIFY(options.loadLastProfile());
    QCOMPARE(options.profileFilePath(), QStringLiteral("profiles/line-b.edserialprofile"));
}

void StartupOptionsTest::lastProfileFlagAfterProfileKeepsExplicitProfilePath()
{
    const StartupOptions options = StartupOptions::fromArguments({
        QStringLiteral("EmbedDebug.exe"),
        QStringLiteral("--profile=profiles/line-a.edserialprofile"),
        QStringLiteral("--last-profile"),
    });

    QCOMPARE(options.panelId(), QStringLiteral("serial.station"));
    QVERIFY(options.loadLastProfile());
    QCOMPARE(options.profileFilePath(), QStringLiteral("profiles/line-a.edserialprofile"));
}

void StartupOptionsTest::lastProfileFalseDoesNotRouteToSerialStation()
{
    const StartupOptions options = StartupOptions::fromArguments({
        QStringLiteral("EmbedDebug.exe"),
        QStringLiteral("--last-profile=false"),
    });

    QCOMPARE(options.panelId(), QString());
    QVERIFY(!options.loadLastProfile());
}

void StartupOptionsTest::serialLastProfileFalseDoesNotRouteToSerialStation()
{
    const StartupOptions options = StartupOptions::fromArguments({
        QStringLiteral("EmbedDebug.exe"),
        QStringLiteral("--serial-last-profile=off"),
    });

    QCOMPARE(options.panelId(), QString());
    QVERIFY(!options.loadLastProfile());
}

void StartupOptionsTest::repeatedLastProfileLikeArgumentsUseLastBooleanValue()
{
    const StartupOptions options = StartupOptions::fromArguments({
        QStringLiteral("EmbedDebug.exe"),
        QStringLiteral("--last-profile"),
        QStringLiteral("--serial-last-profile=false"),
    });

    QCOMPARE(options.panelId(), QString());
    QVERIFY(!options.loadLastProfile());
}

void StartupOptionsTest::unknownLastProfileValueIsIgnored()
{
    const StartupOptions options = StartupOptions::fromArguments({
        QStringLiteral("EmbedDebug.exe"),
        QStringLiteral("--last-profile=maybe"),
    });

    QCOMPARE(options.panelId(), QString());
    QVERIFY(!options.loadLastProfile());
}

void StartupOptionsTest::lastProfileEqualsTrimsValue()
{
    const StartupOptions options = StartupOptions::fromArguments({
        QStringLiteral("EmbedDebug.exe"),
        QStringLiteral("--last-profile=  yes  "),
    });

    QCOMPARE(options.panelId(), QStringLiteral("serial.station"));
    QVERIFY(options.loadLastProfile());
}

void StartupOptionsTest::lastProfileFlagDoesNotConsumeNextArgument()
{
    const StartupOptions options = StartupOptions::fromArguments({
        QStringLiteral("EmbedDebug.exe"),
        QStringLiteral("--last-profile"),
        QStringLiteral("--panel"),
        QStringLiteral("diagnostics.dashboard"),
    });

    QCOMPARE(options.panelId(), QStringLiteral("diagnostics.dashboard"));
    QVERIFY(options.loadLastProfile());
}

void StartupOptionsTest::lastProfileFlagCanAppearBetweenProfileFlagAndValue()
{
    const StartupOptions options = StartupOptions::fromArguments({
        QStringLiteral("EmbedDebug.exe"),
        QStringLiteral("--profile"),
        QStringLiteral("--last-profile"),
        QStringLiteral("profiles/line-a.edserialprofile"),
    });

    QCOMPARE(options.panelId(), QStringLiteral("serial.station"));
    QVERIFY(options.loadLastProfile());
    QCOMPARE(options.profileFilePath(), QString());
}

void StartupOptionsTest::lastProfileEqualsTrueKeepsExplicitPanel()
{
    const StartupOptions options = StartupOptions::fromArguments({
        QStringLiteral("EmbedDebug.exe"),
        QStringLiteral("--last-profile=true"),
        QStringLiteral("--panel=diagnostics.dashboard"),
    });

    QCOMPARE(options.panelId(), QStringLiteral("diagnostics.dashboard"));
    QVERIFY(options.loadLastProfile());
    QCOMPARE(options.profileFilePath(), QString());
}

void StartupOptionsTest::serialLastProfileFlagDoesNotConsumeStationFlag()
{
    const StartupOptions options = StartupOptions::fromArguments({
        QStringLiteral("EmbedDebug.exe"),
        QStringLiteral("--serial-last-profile"),
        QStringLiteral("--station"),
        QStringLiteral("serial"),
    });

    QCOMPARE(options.panelId(), QStringLiteral("serial.station"));
    QVERIFY(options.loadLastProfile());
    QCOMPARE(options.profileFilePath(), QString());
}

QTEST_MAIN(StartupOptionsTest)
#include "test_startup_options.moc"
