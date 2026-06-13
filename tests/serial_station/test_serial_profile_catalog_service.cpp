#include <QtTest/QtTest>

#include "apps/serial_station/services/SerialProfileCatalogService.h"
#include "utils/settings/SettingsManager.h"

using serial_station::SerialProfileCatalogService;

class SerialProfileCatalogServiceTest : public QObject {
    Q_OBJECT

private slots:
    void init();
    void cleanup();
    void startsEmptyAfterClear();
    void recordsLastProfilePath();
    void keepsMostRecentPathFirst();
    void deduplicatesRecordedPaths();
    void trimsAndIgnoresBlankPaths();
    void limitsRecentProfileCount();
    void persistsRecentOrderAcrossInstances();
    void normalizesDuplicateSeparators();
    void clearAllowsRecordingAgain();
    void clearRemovesPersistedCatalog();
};

void SerialProfileCatalogServiceTest::init()
{
    SerialProfileCatalogService catalog;
    catalog.clear();
}

void SerialProfileCatalogServiceTest::cleanup()
{
    SerialProfileCatalogService catalog;
    catalog.clear();
}

void SerialProfileCatalogServiceTest::startsEmptyAfterClear()
{
    SerialProfileCatalogService catalog;

    catalog.clear();

    QVERIFY(catalog.recentProfilePaths().isEmpty());
    QCOMPARE(catalog.lastProfilePath(), QString());
}

void SerialProfileCatalogServiceTest::recordsLastProfilePath()
{
    SerialProfileCatalogService catalog;

    QVERIFY(catalog.recordProfilePath(QStringLiteral("C:/profiles/line-a.edserialprofile")));

    QCOMPARE(catalog.lastProfilePath(), QStringLiteral("C:/profiles/line-a.edserialprofile"));
    QCOMPARE(catalog.recentProfilePaths(), QStringList({QStringLiteral("C:/profiles/line-a.edserialprofile")}));
}

void SerialProfileCatalogServiceTest::keepsMostRecentPathFirst()
{
    SerialProfileCatalogService catalog;

    QVERIFY(catalog.recordProfilePath(QStringLiteral("C:/profiles/line-a.edserialprofile")));
    QVERIFY(catalog.recordProfilePath(QStringLiteral("C:/profiles/line-b.edserialprofile")));

    QCOMPARE(catalog.recentProfilePaths(),
             QStringList({QStringLiteral("C:/profiles/line-b.edserialprofile"),
                          QStringLiteral("C:/profiles/line-a.edserialprofile")}));
    QCOMPARE(catalog.lastProfilePath(), QStringLiteral("C:/profiles/line-b.edserialprofile"));
}

void SerialProfileCatalogServiceTest::deduplicatesRecordedPaths()
{
    SerialProfileCatalogService catalog;

    QVERIFY(catalog.recordProfilePath(QStringLiteral("C:/profiles/line-a.edserialprofile")));
    QVERIFY(catalog.recordProfilePath(QStringLiteral("C:/profiles/line-b.edserialprofile")));
    QVERIFY(catalog.recordProfilePath(QStringLiteral("C:/profiles/line-a.edserialprofile")));

    QCOMPARE(catalog.recentProfilePaths(),
             QStringList({QStringLiteral("C:/profiles/line-a.edserialprofile"),
                          QStringLiteral("C:/profiles/line-b.edserialprofile")}));
}

void SerialProfileCatalogServiceTest::trimsAndIgnoresBlankPaths()
{
    SerialProfileCatalogService catalog;

    QVERIFY(!catalog.recordProfilePath(QStringLiteral("  ")));
    QVERIFY(catalog.recordProfilePath(QStringLiteral("  C:/profiles/line-a.edserialprofile  ")));

    QCOMPARE(catalog.recentProfilePaths(), QStringList({QStringLiteral("C:/profiles/line-a.edserialprofile")}));
    QCOMPARE(catalog.lastProfilePath(), QStringLiteral("C:/profiles/line-a.edserialprofile"));
}

void SerialProfileCatalogServiceTest::limitsRecentProfileCount()
{
    SerialProfileCatalogService catalog;

    for (int i = 0; i < 12; ++i) {
        QVERIFY(catalog.recordProfilePath(QStringLiteral("C:/profiles/line-%1.edserialprofile").arg(i)));
    }

    const QStringList recent = catalog.recentProfilePaths();
    QCOMPARE(recent.size(), 8);
    QCOMPARE(recent.first(), QStringLiteral("C:/profiles/line-11.edserialprofile"));
    QCOMPARE(recent.last(), QStringLiteral("C:/profiles/line-4.edserialprofile"));
}

void SerialProfileCatalogServiceTest::persistsRecentOrderAcrossInstances()
{
    {
        SerialProfileCatalogService catalog;
        QVERIFY(catalog.recordProfilePath(QStringLiteral("C:/profiles/line-a.edserialprofile")));
        QVERIFY(catalog.recordProfilePath(QStringLiteral("C:/profiles/line-b.edserialprofile")));
        QVERIFY(catalog.recordProfilePath(QStringLiteral("C:/profiles/line-c.edserialprofile")));
    }

    SerialProfileCatalogService reloadedCatalog;

    QCOMPARE(reloadedCatalog.recentProfilePaths(),
             QStringList({QStringLiteral("C:/profiles/line-c.edserialprofile"),
                          QStringLiteral("C:/profiles/line-b.edserialprofile"),
                          QStringLiteral("C:/profiles/line-a.edserialprofile")}));
    QCOMPARE(reloadedCatalog.lastProfilePath(), QStringLiteral("C:/profiles/line-c.edserialprofile"));
}

void SerialProfileCatalogServiceTest::normalizesDuplicateSeparators()
{
    SerialProfileCatalogService catalog;

    QVERIFY(catalog.recordProfilePath(QStringLiteral("C:/profiles//line-a.edserialprofile")));
    QVERIFY(catalog.recordProfilePath(QStringLiteral("C:/profiles/line-b.edserialprofile")));
    QVERIFY(catalog.recordProfilePath(QStringLiteral("C:/profiles/./line-a.edserialprofile")));

    QCOMPARE(catalog.recentProfilePaths(),
             QStringList({QStringLiteral("C:/profiles/line-a.edserialprofile"),
                          QStringLiteral("C:/profiles/line-b.edserialprofile")}));
    QCOMPARE(catalog.lastProfilePath(), QStringLiteral("C:/profiles/line-a.edserialprofile"));
}

void SerialProfileCatalogServiceTest::clearAllowsRecordingAgain()
{
    SerialProfileCatalogService catalog;

    QVERIFY(catalog.recordProfilePath(QStringLiteral("C:/profiles/line-a.edserialprofile")));
    catalog.clear();
    QVERIFY(catalog.recordProfilePath(QStringLiteral("C:/profiles/line-b.edserialprofile")));

    QCOMPARE(catalog.recentProfilePaths(), QStringList({QStringLiteral("C:/profiles/line-b.edserialprofile")}));
    QCOMPARE(catalog.lastProfilePath(), QStringLiteral("C:/profiles/line-b.edserialprofile"));
}

void SerialProfileCatalogServiceTest::clearRemovesPersistedCatalog()
{
    {
        SerialProfileCatalogService catalog;
        QVERIFY(catalog.recordProfilePath(QStringLiteral("C:/profiles/line-a.edserialprofile")));
    }

    SerialProfileCatalogService reloadedCatalog;
    QCOMPARE(reloadedCatalog.lastProfilePath(), QStringLiteral("C:/profiles/line-a.edserialprofile"));

    reloadedCatalog.clear();

    SerialProfileCatalogService clearedCatalog;
    QVERIFY(clearedCatalog.recentProfilePaths().isEmpty());
    QCOMPARE(clearedCatalog.lastProfilePath(), QString());
}

QTEST_GUILESS_MAIN(SerialProfileCatalogServiceTest)
#include "test_serial_profile_catalog_service.moc"
