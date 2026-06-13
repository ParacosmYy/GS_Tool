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
    void containsProfilePathUsesNormalizedPaths();
    void containsProfilePathRejectsBlankInput();
    void removeExistingProfilePathUpdatesRecentList();
    void removeMissingProfilePathKeepsCatalog();
    void removeLastProfileFallsBackToNextRecent();
    void removeOnlyProfileClearsLastProfile();
    void removeProfilePathPersistsAcrossInstances();
    void removeProfilePathAllowsRecordingAgain();
    void removeProfilePathRejectsBlankInput();
    void removeOlderProfileKeepsCurrentLastProfile();
    void removeProfilePathNormalizesInput();
    void removeProfilePathHandlesLegacyPipeSeparatedCatalog();
    void removeProfilePathPrunesDuplicateStoredEntries();
    void removeProfilePathPreservesMaxRecentLimit();
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

void SerialProfileCatalogServiceTest::containsProfilePathUsesNormalizedPaths()
{
    SerialProfileCatalogService catalog;

    QVERIFY(catalog.recordProfilePath(QStringLiteral("C:/profiles//line-a.edserialprofile")));

    QVERIFY(catalog.containsProfilePath(QStringLiteral(" C:/profiles/./line-a.edserialprofile ")));
    QVERIFY(!catalog.containsProfilePath(QStringLiteral("C:/profiles/line-b.edserialprofile")));
}

void SerialProfileCatalogServiceTest::containsProfilePathRejectsBlankInput()
{
    SerialProfileCatalogService catalog;

    QVERIFY(catalog.recordProfilePath(QStringLiteral("C:/profiles/line-a.edserialprofile")));

    QVERIFY(!catalog.containsProfilePath(QString()));
    QVERIFY(!catalog.containsProfilePath(QStringLiteral("   ")));
}

void SerialProfileCatalogServiceTest::removeExistingProfilePathUpdatesRecentList()
{
    SerialProfileCatalogService catalog;

    QVERIFY(catalog.recordProfilePath(QStringLiteral("C:/profiles/line-a.edserialprofile")));
    QVERIFY(catalog.recordProfilePath(QStringLiteral("C:/profiles/line-b.edserialprofile")));
    QVERIFY(catalog.recordProfilePath(QStringLiteral("C:/profiles/line-c.edserialprofile")));

    QVERIFY(catalog.removeProfilePath(QStringLiteral("C:/profiles/line-b.edserialprofile")));

    QCOMPARE(catalog.recentProfilePaths(),
             QStringList({QStringLiteral("C:/profiles/line-c.edserialprofile"),
                          QStringLiteral("C:/profiles/line-a.edserialprofile")}));
    QVERIFY(!catalog.containsProfilePath(QStringLiteral("C:/profiles/line-b.edserialprofile")));
}

void SerialProfileCatalogServiceTest::removeMissingProfilePathKeepsCatalog()
{
    SerialProfileCatalogService catalog;

    QVERIFY(catalog.recordProfilePath(QStringLiteral("C:/profiles/line-a.edserialprofile")));
    QVERIFY(catalog.recordProfilePath(QStringLiteral("C:/profiles/line-b.edserialprofile")));
    const QStringList before = catalog.recentProfilePaths();

    QVERIFY(!catalog.removeProfilePath(QStringLiteral("C:/profiles/missing.edserialprofile")));

    QCOMPARE(catalog.recentProfilePaths(), before);
    QCOMPARE(catalog.lastProfilePath(), QStringLiteral("C:/profiles/line-b.edserialprofile"));
}

void SerialProfileCatalogServiceTest::removeLastProfileFallsBackToNextRecent()
{
    SerialProfileCatalogService catalog;

    QVERIFY(catalog.recordProfilePath(QStringLiteral("C:/profiles/line-a.edserialprofile")));
    QVERIFY(catalog.recordProfilePath(QStringLiteral("C:/profiles/line-b.edserialprofile")));
    QVERIFY(catalog.recordProfilePath(QStringLiteral("C:/profiles/line-c.edserialprofile")));

    QVERIFY(catalog.removeProfilePath(QStringLiteral("C:/profiles/line-c.edserialprofile")));

    QCOMPARE(catalog.lastProfilePath(), QStringLiteral("C:/profiles/line-b.edserialprofile"));
    QCOMPARE(catalog.recentProfilePaths(),
             QStringList({QStringLiteral("C:/profiles/line-b.edserialprofile"),
                          QStringLiteral("C:/profiles/line-a.edserialprofile")}));
}

void SerialProfileCatalogServiceTest::removeOnlyProfileClearsLastProfile()
{
    SerialProfileCatalogService catalog;

    QVERIFY(catalog.recordProfilePath(QStringLiteral("C:/profiles/line-a.edserialprofile")));

    QVERIFY(catalog.removeProfilePath(QStringLiteral("C:/profiles/line-a.edserialprofile")));

    QVERIFY(catalog.recentProfilePaths().isEmpty());
    QCOMPARE(catalog.lastProfilePath(), QString());
}

void SerialProfileCatalogServiceTest::removeProfilePathPersistsAcrossInstances()
{
    {
        SerialProfileCatalogService catalog;
        QVERIFY(catalog.recordProfilePath(QStringLiteral("C:/profiles/line-a.edserialprofile")));
        QVERIFY(catalog.recordProfilePath(QStringLiteral("C:/profiles/line-b.edserialprofile")));
        QVERIFY(catalog.removeProfilePath(QStringLiteral("C:/profiles/line-b.edserialprofile")));
    }

    SerialProfileCatalogService reloadedCatalog;

    QCOMPARE(reloadedCatalog.recentProfilePaths(),
             QStringList({QStringLiteral("C:/profiles/line-a.edserialprofile")}));
    QCOMPARE(reloadedCatalog.lastProfilePath(), QStringLiteral("C:/profiles/line-a.edserialprofile"));
}

void SerialProfileCatalogServiceTest::removeProfilePathAllowsRecordingAgain()
{
    SerialProfileCatalogService catalog;

    QVERIFY(catalog.recordProfilePath(QStringLiteral("C:/profiles/line-a.edserialprofile")));
    QVERIFY(catalog.removeProfilePath(QStringLiteral("C:/profiles/line-a.edserialprofile")));
    QVERIFY(catalog.recordProfilePath(QStringLiteral("C:/profiles/line-a.edserialprofile")));

    QCOMPARE(catalog.recentProfilePaths(), QStringList({QStringLiteral("C:/profiles/line-a.edserialprofile")}));
    QCOMPARE(catalog.lastProfilePath(), QStringLiteral("C:/profiles/line-a.edserialprofile"));
}

void SerialProfileCatalogServiceTest::removeProfilePathRejectsBlankInput()
{
    SerialProfileCatalogService catalog;

    QVERIFY(catalog.recordProfilePath(QStringLiteral("C:/profiles/line-a.edserialprofile")));

    QVERIFY(!catalog.removeProfilePath(QString()));
    QVERIFY(!catalog.removeProfilePath(QStringLiteral("  ")));
    QCOMPARE(catalog.recentProfilePaths(), QStringList({QStringLiteral("C:/profiles/line-a.edserialprofile")}));
}

void SerialProfileCatalogServiceTest::removeOlderProfileKeepsCurrentLastProfile()
{
    SerialProfileCatalogService catalog;

    QVERIFY(catalog.recordProfilePath(QStringLiteral("C:/profiles/line-a.edserialprofile")));
    QVERIFY(catalog.recordProfilePath(QStringLiteral("C:/profiles/line-b.edserialprofile")));
    QVERIFY(catalog.recordProfilePath(QStringLiteral("C:/profiles/line-c.edserialprofile")));

    QVERIFY(catalog.removeProfilePath(QStringLiteral("C:/profiles/line-a.edserialprofile")));

    QCOMPARE(catalog.lastProfilePath(), QStringLiteral("C:/profiles/line-c.edserialprofile"));
    QCOMPARE(catalog.recentProfilePaths(),
             QStringList({QStringLiteral("C:/profiles/line-c.edserialprofile"),
                          QStringLiteral("C:/profiles/line-b.edserialprofile")}));
}

void SerialProfileCatalogServiceTest::removeProfilePathNormalizesInput()
{
    SerialProfileCatalogService catalog;

    QVERIFY(catalog.recordProfilePath(QStringLiteral("C:/profiles//line-a.edserialprofile")));
    QVERIFY(catalog.recordProfilePath(QStringLiteral("C:/profiles/line-b.edserialprofile")));

    QVERIFY(catalog.removeProfilePath(QStringLiteral(" C:/profiles/./line-a.edserialprofile ")));

    QCOMPARE(catalog.recentProfilePaths(), QStringList({QStringLiteral("C:/profiles/line-b.edserialprofile")}));
    QVERIFY(!catalog.containsProfilePath(QStringLiteral("C:/profiles/line-a.edserialprofile")));
}

void SerialProfileCatalogServiceTest::removeProfilePathHandlesLegacyPipeSeparatedCatalog()
{
    SettingsManager::instance().set(
        QStringLiteral("serial_station/profiles/recent"),
        QStringLiteral("C:/profiles/line-a.edserialprofile|C:/profiles/line-b.edserialprofile"));
    SettingsManager::instance().set(QStringLiteral("serial_station/profiles/last"),
                                    QStringLiteral("C:/profiles/line-a.edserialprofile"));
    SettingsManager::instance().sync();

    SerialProfileCatalogService catalog;

    QVERIFY(catalog.removeProfilePath(QStringLiteral("C:/profiles/line-a.edserialprofile")));

    QCOMPARE(catalog.recentProfilePaths(), QStringList({QStringLiteral("C:/profiles/line-b.edserialprofile")}));
    QCOMPARE(catalog.lastProfilePath(), QStringLiteral("C:/profiles/line-b.edserialprofile"));
}

void SerialProfileCatalogServiceTest::removeProfilePathPrunesDuplicateStoredEntries()
{
    SettingsManager::instance().set(
        QStringLiteral("serial_station/profiles/recent"),
        QStringList({QStringLiteral("C:/profiles/line-a.edserialprofile"),
                     QStringLiteral("C:/profiles/./line-a.edserialprofile"),
                     QStringLiteral("C:/profiles/line-b.edserialprofile")}));
    SettingsManager::instance().set(QStringLiteral("serial_station/profiles/last"),
                                    QStringLiteral("C:/profiles/line-a.edserialprofile"));
    SettingsManager::instance().sync();

    SerialProfileCatalogService catalog;

    QVERIFY(catalog.removeProfilePath(QStringLiteral("C:/profiles/line-a.edserialprofile")));

    QCOMPARE(catalog.recentProfilePaths(), QStringList({QStringLiteral("C:/profiles/line-b.edserialprofile")}));
    QCOMPARE(catalog.lastProfilePath(), QStringLiteral("C:/profiles/line-b.edserialprofile"));
}

void SerialProfileCatalogServiceTest::removeProfilePathPreservesMaxRecentLimit()
{
    SerialProfileCatalogService catalog;

    for (int i = 0; i < 12; ++i) {
        QVERIFY(catalog.recordProfilePath(QStringLiteral("C:/profiles/line-%1.edserialprofile").arg(i)));
    }

    QVERIFY(catalog.removeProfilePath(QStringLiteral("C:/profiles/line-8.edserialprofile")));

    const QStringList recent = catalog.recentProfilePaths();
    QCOMPARE(recent.size(), 7);
    QVERIFY(!recent.contains(QStringLiteral("C:/profiles/line-8.edserialprofile")));
    QCOMPARE(recent.first(), QStringLiteral("C:/profiles/line-11.edserialprofile"));
    QCOMPARE(recent.last(), QStringLiteral("C:/profiles/line-4.edserialprofile"));
}

QTEST_GUILESS_MAIN(SerialProfileCatalogServiceTest)
#include "test_serial_profile_catalog_service.moc"
