#include <QtTest/QtTest>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QTemporaryDir>

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
    void pruneMissingProfilePathsRemovesOnlyMissingFiles();
    void pruneMissingProfilePathsReturnsRemovedCount();
    void pruneMissingProfilePathsUpdatesLastToFirstExisting();
    void pruneMissingProfilePathsClearsLastWhenNoneExist();
    void pruneMissingProfilePathsPersistsAcrossInstances();
    void pruneMissingProfilePathsKeepsOrderOfExistingProfiles();
    void pruneMissingProfilePathsIsIdempotent();
    void pruneMissingProfilePathsNormalizesStoredPaths();
    void pruneMissingProfilePathsHandlesEmptyCatalog();
    void pruneMissingProfilePathsRetainsLastWhenStillExisting();
    void pruneMissingProfilePathsClearsStrayLastWithoutRecentProfiles();
    void defaultProfileDirectoryStartsEmpty();
    void setDefaultProfileDirectoryPersistsNormalizedPath();
    void setDefaultProfileDirectoryRejectsBlankPath();
    void clearDefaultProfileDirectoryRemovesStoredPath();
    void clearRemovesDefaultProfileDirectory();
    void defaultProfileDirectoryPersistsAcrossInstances();
    void setDefaultProfileDirectoryDoesNotChangeRecentProfiles();
    void setDefaultProfileDirectoryHandlesDuplicateSeparators();
    void clearDefaultProfileDirectoryKeepsRecentProfiles();
    void defaultProfileDirectoryCanBeReplaced();
    void blankDefaultProfileDirectoryDoesNotCreateStoredValue();
    void clearDefaultProfileDirectoryPersistsAcrossInstances();
    void recordProfilePathDoesNotChangeDefaultDirectory();
    void removeProfilePathDoesNotChangeDefaultDirectory();
    void pruneMissingProfilePathsDoesNotChangeDefaultDirectory();
    void defaultProfileDirectoryNormalizesStoredRawValue();
    void setDefaultProfileDirectoryAcceptsRelativePath();
    void setDefaultProfileDirectoryCollapsesParentSegments();
    void clearDefaultProfileDirectoryIsIdempotent();
    void defaultProfileDirectoryKeepsLastProfilePathIndependent();
    void discoverProfilePathsReturnsEmptyForBlankDirectory();
    void discoverProfilePathsReturnsEmptyForMissingDirectory();
    void discoverProfilePathsFindsSupportedExtensionsOnly();
    void discoverProfilePathsSortsByFileName();
    void discoverProfilePathsDoesNotModifyCatalog();
    void discoverProfilePathsDoesNotScanSubdirectories();
    void importProfileDirectoryRecordsDiscoveredProfiles();
    void importProfileDirectoryReturnsOnlyNewlyImportedCount();
    void importProfileDirectoryKeepsDefaultDirectory();
    void importProfileDirectoryRespectsRecentLimit();
    void importProfileDirectoryPersistsAcrossInstances();
    void importProfileDirectoryHandlesJsonProfiles();
    void discoverProfilePathsTrimsDirectoryInput();
    void discoverProfilePathsKeepsCatalogDefaultDirectory();
    void importMissingProfileDirectoryReturnsZero();
    void importEmptyProfileDirectoryKeepsExistingCatalog();
    void importProfileDirectoryKeepsUnrelatedRecentProfiles();
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

void SerialProfileCatalogServiceTest::pruneMissingProfilePathsRemovesOnlyMissingFiles()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString existingPath = QDir(tempDir.path()).filePath(QStringLiteral("line-a.edserialprofile"));
    const QString missingPath = QDir(tempDir.path()).filePath(QStringLiteral("missing.edserialprofile"));
    QFile file(existingPath);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("{}");
    file.close();

    SerialProfileCatalogService catalog;
    QVERIFY(catalog.recordProfilePath(missingPath));
    QVERIFY(catalog.recordProfilePath(existingPath));

    QCOMPARE(catalog.pruneMissingProfilePaths(), 1);
    QCOMPARE(catalog.recentProfilePaths(), QStringList({existingPath}));
    QCOMPARE(catalog.lastProfilePath(), existingPath);
}

void SerialProfileCatalogServiceTest::pruneMissingProfilePathsReturnsRemovedCount()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString existingPath = QDir(tempDir.path()).filePath(QStringLiteral("line-a.edserialprofile"));
    const QString missingA = QDir(tempDir.path()).filePath(QStringLiteral("missing-a.edserialprofile"));
    const QString missingB = QDir(tempDir.path()).filePath(QStringLiteral("missing-b.edserialprofile"));
    QFile file(existingPath);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.close();

    SerialProfileCatalogService catalog;
    QVERIFY(catalog.recordProfilePath(existingPath));
    QVERIFY(catalog.recordProfilePath(missingA));
    QVERIFY(catalog.recordProfilePath(missingB));

    QCOMPARE(catalog.pruneMissingProfilePaths(), 2);
    QCOMPARE(catalog.recentProfilePaths(), QStringList({existingPath}));
}

void SerialProfileCatalogServiceTest::pruneMissingProfilePathsUpdatesLastToFirstExisting()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString fallbackPath = QDir(tempDir.path()).filePath(QStringLiteral("fallback.edserialprofile"));
    const QString missingPath = QDir(tempDir.path()).filePath(QStringLiteral("last-missing.edserialprofile"));
    QFile file(fallbackPath);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.close();

    SerialProfileCatalogService catalog;
    QVERIFY(catalog.recordProfilePath(fallbackPath));
    QVERIFY(catalog.recordProfilePath(missingPath));

    QCOMPARE(catalog.lastProfilePath(), missingPath);
    QCOMPARE(catalog.pruneMissingProfilePaths(), 1);
    QCOMPARE(catalog.lastProfilePath(), fallbackPath);
}

void SerialProfileCatalogServiceTest::pruneMissingProfilePathsClearsLastWhenNoneExist()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString missingPath = QDir(tempDir.path()).filePath(QStringLiteral("missing.edserialprofile"));

    SerialProfileCatalogService catalog;
    QVERIFY(catalog.recordProfilePath(missingPath));

    QCOMPARE(catalog.pruneMissingProfilePaths(), 1);
    QVERIFY(catalog.recentProfilePaths().isEmpty());
    QCOMPARE(catalog.lastProfilePath(), QString());
}

void SerialProfileCatalogServiceTest::pruneMissingProfilePathsPersistsAcrossInstances()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString existingPath = QDir(tempDir.path()).filePath(QStringLiteral("line-a.edserialprofile"));
    const QString missingPath = QDir(tempDir.path()).filePath(QStringLiteral("missing.edserialprofile"));
    QFile file(existingPath);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.close();

    {
        SerialProfileCatalogService catalog;
        QVERIFY(catalog.recordProfilePath(existingPath));
        QVERIFY(catalog.recordProfilePath(missingPath));
        QCOMPARE(catalog.pruneMissingProfilePaths(), 1);
    }

    SerialProfileCatalogService reloadedCatalog;
    QCOMPARE(reloadedCatalog.recentProfilePaths(), QStringList({existingPath}));
    QCOMPARE(reloadedCatalog.lastProfilePath(), existingPath);
}

void SerialProfileCatalogServiceTest::pruneMissingProfilePathsKeepsOrderOfExistingProfiles()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString firstPath = QDir(tempDir.path()).filePath(QStringLiteral("first.edserialprofile"));
    const QString secondPath = QDir(tempDir.path()).filePath(QStringLiteral("second.edserialprofile"));
    const QString missingPath = QDir(tempDir.path()).filePath(QStringLiteral("missing.edserialprofile"));
    const QStringList existingPaths = {firstPath, secondPath};
    for (const QString& path : existingPaths) {
        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.close();
    }

    SerialProfileCatalogService catalog;
    QVERIFY(catalog.recordProfilePath(secondPath));
    QVERIFY(catalog.recordProfilePath(missingPath));
    QVERIFY(catalog.recordProfilePath(firstPath));

    QCOMPARE(catalog.pruneMissingProfilePaths(), 1);
    QCOMPARE(catalog.recentProfilePaths(), QStringList({firstPath, secondPath}));
}

void SerialProfileCatalogServiceTest::pruneMissingProfilePathsIsIdempotent()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString existingPath = QDir(tempDir.path()).filePath(QStringLiteral("line-a.edserialprofile"));
    QFile file(existingPath);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.close();

    SerialProfileCatalogService catalog;
    QVERIFY(catalog.recordProfilePath(existingPath));

    QCOMPARE(catalog.pruneMissingProfilePaths(), 0);
    QCOMPARE(catalog.pruneMissingProfilePaths(), 0);
    QCOMPARE(catalog.recentProfilePaths(), QStringList({existingPath}));
}

void SerialProfileCatalogServiceTest::pruneMissingProfilePathsNormalizesStoredPaths()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString existingPath = QDir(tempDir.path()).filePath(QStringLiteral("line-a.edserialprofile"));
    QFile file(existingPath);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.close();

    SerialProfileCatalogService catalog;
    QVERIFY(catalog.recordProfilePath(QDir(tempDir.path()).filePath(QStringLiteral("./line-a.edserialprofile"))));

    QCOMPARE(catalog.pruneMissingProfilePaths(), 0);
    QCOMPARE(catalog.recentProfilePaths(), QStringList({existingPath}));
}

void SerialProfileCatalogServiceTest::pruneMissingProfilePathsHandlesEmptyCatalog()
{
    SerialProfileCatalogService catalog;

    QCOMPARE(catalog.pruneMissingProfilePaths(), 0);
    QVERIFY(catalog.recentProfilePaths().isEmpty());
    QCOMPARE(catalog.lastProfilePath(), QString());
}

void SerialProfileCatalogServiceTest::pruneMissingProfilePathsRetainsLastWhenStillExisting()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString lastPath = QDir(tempDir.path()).filePath(QStringLiteral("last.edserialprofile"));
    const QString olderPath = QDir(tempDir.path()).filePath(QStringLiteral("older.edserialprofile"));
    const QStringList existingPaths = {lastPath, olderPath};
    for (const QString& path : existingPaths) {
        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.close();
    }

    SerialProfileCatalogService catalog;
    QVERIFY(catalog.recordProfilePath(olderPath));
    QVERIFY(catalog.recordProfilePath(lastPath));

    QCOMPARE(catalog.pruneMissingProfilePaths(), 0);
    QCOMPARE(catalog.lastProfilePath(), lastPath);
    QCOMPARE(catalog.recentProfilePaths(), QStringList({lastPath, olderPath}));
}

void SerialProfileCatalogServiceTest::pruneMissingProfilePathsClearsStrayLastWithoutRecentProfiles()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString missingPath = QDir(tempDir.path()).filePath(QStringLiteral("stale-last.edserialprofile"));
    SettingsManager::instance().set(QStringLiteral("serial_station/profiles/last"), missingPath);
    SettingsManager::instance().sync();

    SerialProfileCatalogService catalog;

    QCOMPARE(catalog.pruneMissingProfilePaths(), 0);
    QVERIFY(catalog.recentProfilePaths().isEmpty());
    QCOMPARE(catalog.lastProfilePath(), QString());
}

void SerialProfileCatalogServiceTest::defaultProfileDirectoryStartsEmpty()
{
    SerialProfileCatalogService catalog;

    QCOMPARE(catalog.defaultProfileDirectory(), QString());
}

void SerialProfileCatalogServiceTest::setDefaultProfileDirectoryPersistsNormalizedPath()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QDir dir(tempDir.path());
    QVERIFY(dir.mkpath(QStringLiteral("profiles/line-a")));
    const QString rawPath = dir.filePath(QStringLiteral("profiles/./line-a"));
    const QString expectedPath = QDir::cleanPath(rawPath);

    SerialProfileCatalogService catalog;
    QVERIFY(catalog.setDefaultProfileDirectory(rawPath));

    QCOMPARE(catalog.defaultProfileDirectory(), expectedPath);
}

void SerialProfileCatalogServiceTest::setDefaultProfileDirectoryRejectsBlankPath()
{
    SerialProfileCatalogService catalog;
    QVERIFY(catalog.setDefaultProfileDirectory(QStringLiteral("C:/profiles/line-a")));

    QVERIFY(!catalog.setDefaultProfileDirectory(QStringLiteral("   ")));

    QCOMPARE(catalog.defaultProfileDirectory(), QStringLiteral("C:/profiles/line-a"));
}

void SerialProfileCatalogServiceTest::clearDefaultProfileDirectoryRemovesStoredPath()
{
    SerialProfileCatalogService catalog;
    QVERIFY(catalog.setDefaultProfileDirectory(QStringLiteral("C:/profiles/line-a")));

    catalog.clearDefaultProfileDirectory();

    QCOMPARE(catalog.defaultProfileDirectory(), QString());
}

void SerialProfileCatalogServiceTest::clearRemovesDefaultProfileDirectory()
{
    SerialProfileCatalogService catalog;
    QVERIFY(catalog.setDefaultProfileDirectory(QStringLiteral("C:/profiles/line-a")));
    QVERIFY(catalog.recordProfilePath(QStringLiteral("C:/profiles/line-a/profile.edserialprofile")));

    catalog.clear();

    QCOMPARE(catalog.defaultProfileDirectory(), QString());
    QVERIFY(catalog.recentProfilePaths().isEmpty());
    QCOMPARE(catalog.lastProfilePath(), QString());
}

void SerialProfileCatalogServiceTest::defaultProfileDirectoryPersistsAcrossInstances()
{
    {
        SerialProfileCatalogService catalog;
        QVERIFY(catalog.setDefaultProfileDirectory(QStringLiteral("C:/profiles/line-a")));
    }

    SerialProfileCatalogService reloadedCatalog;

    QCOMPARE(reloadedCatalog.defaultProfileDirectory(), QStringLiteral("C:/profiles/line-a"));
}

void SerialProfileCatalogServiceTest::setDefaultProfileDirectoryDoesNotChangeRecentProfiles()
{
    SerialProfileCatalogService catalog;
    QVERIFY(catalog.recordProfilePath(QStringLiteral("C:/profiles/line-a/profile.edserialprofile")));
    QVERIFY(catalog.recordProfilePath(QStringLiteral("C:/profiles/line-b/profile.edserialprofile")));

    QVERIFY(catalog.setDefaultProfileDirectory(QStringLiteral("C:/profiles/line-c")));

    QCOMPARE(catalog.defaultProfileDirectory(), QStringLiteral("C:/profiles/line-c"));
    QCOMPARE(catalog.recentProfilePaths(),
             QStringList({QStringLiteral("C:/profiles/line-b/profile.edserialprofile"),
                          QStringLiteral("C:/profiles/line-a/profile.edserialprofile")}));
    QCOMPARE(catalog.lastProfilePath(), QStringLiteral("C:/profiles/line-b/profile.edserialprofile"));
}

void SerialProfileCatalogServiceTest::setDefaultProfileDirectoryHandlesDuplicateSeparators()
{
    SerialProfileCatalogService catalog;

    QVERIFY(catalog.setDefaultProfileDirectory(QStringLiteral(" C:/profiles//line-a/./profiles ")));

    QCOMPARE(catalog.defaultProfileDirectory(), QStringLiteral("C:/profiles/line-a/profiles"));
}

void SerialProfileCatalogServiceTest::clearDefaultProfileDirectoryKeepsRecentProfiles()
{
    SerialProfileCatalogService catalog;
    QVERIFY(catalog.recordProfilePath(QStringLiteral("C:/profiles/line-a/profile.edserialprofile")));
    QVERIFY(catalog.setDefaultProfileDirectory(QStringLiteral("C:/profiles/line-a")));

    catalog.clearDefaultProfileDirectory();

    QCOMPARE(catalog.defaultProfileDirectory(), QString());
    QCOMPARE(catalog.recentProfilePaths(),
             QStringList({QStringLiteral("C:/profiles/line-a/profile.edserialprofile")}));
    QCOMPARE(catalog.lastProfilePath(), QStringLiteral("C:/profiles/line-a/profile.edserialprofile"));
}

void SerialProfileCatalogServiceTest::defaultProfileDirectoryCanBeReplaced()
{
    SerialProfileCatalogService catalog;
    QVERIFY(catalog.setDefaultProfileDirectory(QStringLiteral("C:/profiles/line-a")));
    QVERIFY(catalog.setDefaultProfileDirectory(QStringLiteral("C:/profiles/line-b")));

    QCOMPARE(catalog.defaultProfileDirectory(), QStringLiteral("C:/profiles/line-b"));
}

void SerialProfileCatalogServiceTest::blankDefaultProfileDirectoryDoesNotCreateStoredValue()
{
    SerialProfileCatalogService catalog;

    QVERIFY(!catalog.setDefaultProfileDirectory(QStringLiteral("   ")));

    QCOMPARE(catalog.defaultProfileDirectory(), QString());
}

void SerialProfileCatalogServiceTest::clearDefaultProfileDirectoryPersistsAcrossInstances()
{
    {
        SerialProfileCatalogService catalog;
        QVERIFY(catalog.setDefaultProfileDirectory(QStringLiteral("C:/profiles/line-a")));
        catalog.clearDefaultProfileDirectory();
    }

    SerialProfileCatalogService reloadedCatalog;

    QCOMPARE(reloadedCatalog.defaultProfileDirectory(), QString());
}

void SerialProfileCatalogServiceTest::recordProfilePathDoesNotChangeDefaultDirectory()
{
    SerialProfileCatalogService catalog;
    QVERIFY(catalog.setDefaultProfileDirectory(QStringLiteral("C:/profiles/default")));

    QVERIFY(catalog.recordProfilePath(QStringLiteral("C:/profiles/line-a/profile.edserialprofile")));

    QCOMPARE(catalog.defaultProfileDirectory(), QStringLiteral("C:/profiles/default"));
    QCOMPARE(catalog.lastProfilePath(), QStringLiteral("C:/profiles/line-a/profile.edserialprofile"));
}

void SerialProfileCatalogServiceTest::removeProfilePathDoesNotChangeDefaultDirectory()
{
    SerialProfileCatalogService catalog;
    QVERIFY(catalog.setDefaultProfileDirectory(QStringLiteral("C:/profiles/default")));
    QVERIFY(catalog.recordProfilePath(QStringLiteral("C:/profiles/line-a/profile.edserialprofile")));

    QVERIFY(catalog.removeProfilePath(QStringLiteral("C:/profiles/line-a/profile.edserialprofile")));

    QCOMPARE(catalog.defaultProfileDirectory(), QStringLiteral("C:/profiles/default"));
    QVERIFY(catalog.recentProfilePaths().isEmpty());
}

void SerialProfileCatalogServiceTest::pruneMissingProfilePathsDoesNotChangeDefaultDirectory()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString missingPath = QDir(tempDir.path()).filePath(QStringLiteral("missing.edserialprofile"));

    SerialProfileCatalogService catalog;
    QVERIFY(catalog.setDefaultProfileDirectory(QDir(tempDir.path()).filePath(QStringLiteral("profiles"))));
    QVERIFY(catalog.recordProfilePath(missingPath));

    QCOMPARE(catalog.pruneMissingProfilePaths(), 1);

    QCOMPARE(catalog.defaultProfileDirectory(),
             QDir::cleanPath(QDir(tempDir.path()).filePath(QStringLiteral("profiles"))));
}

void SerialProfileCatalogServiceTest::defaultProfileDirectoryNormalizesStoredRawValue()
{
    SettingsManager::instance().set(QStringLiteral("serial_station/profiles/defaultDirectory"),
                                    QStringLiteral(" C:/profiles//line-a/./profiles "));
    SettingsManager::instance().sync();

    SerialProfileCatalogService catalog;

    QCOMPARE(catalog.defaultProfileDirectory(), QStringLiteral("C:/profiles/line-a/profiles"));
}

void SerialProfileCatalogServiceTest::setDefaultProfileDirectoryAcceptsRelativePath()
{
    SerialProfileCatalogService catalog;

    QVERIFY(catalog.setDefaultProfileDirectory(QStringLiteral("./profiles/line-a")));

    QCOMPARE(catalog.defaultProfileDirectory(), QStringLiteral("profiles/line-a"));
}

void SerialProfileCatalogServiceTest::setDefaultProfileDirectoryCollapsesParentSegments()
{
    SerialProfileCatalogService catalog;

    QVERIFY(catalog.setDefaultProfileDirectory(QStringLiteral("C:/factory/profiles/../line-a")));

    QCOMPARE(catalog.defaultProfileDirectory(), QStringLiteral("C:/factory/line-a"));
}

void SerialProfileCatalogServiceTest::clearDefaultProfileDirectoryIsIdempotent()
{
    SerialProfileCatalogService catalog;

    catalog.clearDefaultProfileDirectory();
    catalog.clearDefaultProfileDirectory();

    QCOMPARE(catalog.defaultProfileDirectory(), QString());
}

void SerialProfileCatalogServiceTest::defaultProfileDirectoryKeepsLastProfilePathIndependent()
{
    SerialProfileCatalogService catalog;
    QVERIFY(catalog.recordProfilePath(QStringLiteral("C:/profiles/line-a/profile.edserialprofile")));
    QVERIFY(catalog.setDefaultProfileDirectory(QStringLiteral("C:/profiles/default")));

    catalog.clearDefaultProfileDirectory();

    QCOMPARE(catalog.defaultProfileDirectory(), QString());
    QCOMPARE(catalog.lastProfilePath(), QStringLiteral("C:/profiles/line-a/profile.edserialprofile"));
}

void SerialProfileCatalogServiceTest::discoverProfilePathsReturnsEmptyForBlankDirectory()
{
    SerialProfileCatalogService catalog;

    QVERIFY(catalog.discoverProfilePaths(QStringLiteral("   ")).isEmpty());
}

void SerialProfileCatalogServiceTest::discoverProfilePathsReturnsEmptyForMissingDirectory()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    SerialProfileCatalogService catalog;

    QVERIFY(catalog.discoverProfilePaths(QDir(tempDir.path()).filePath(QStringLiteral("missing"))).isEmpty());
}

void SerialProfileCatalogServiceTest::discoverProfilePathsFindsSupportedExtensionsOnly()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QDir dir(tempDir.path());
    QFile profileA(dir.filePath(QStringLiteral("line-a.edserialprofile")));
    QVERIFY(profileA.open(QIODevice::WriteOnly));
    profileA.close();
    QFile profileB(dir.filePath(QStringLiteral("line-b.json")));
    QVERIFY(profileB.open(QIODevice::WriteOnly));
    profileB.close();
    QFile ignored(dir.filePath(QStringLiteral("notes.txt")));
    QVERIFY(ignored.open(QIODevice::WriteOnly));
    ignored.close();

    SerialProfileCatalogService catalog;

    QCOMPARE(catalog.discoverProfilePaths(tempDir.path()),
             QStringList({dir.absoluteFilePath(QStringLiteral("line-a.edserialprofile")),
                          dir.absoluteFilePath(QStringLiteral("line-b.json"))}));
}

void SerialProfileCatalogServiceTest::discoverProfilePathsSortsByFileName()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QDir dir(tempDir.path());
    const QStringList names = {
        QStringLiteral("zeta.edserialprofile"),
        QStringLiteral("alpha.edserialprofile"),
        QStringLiteral("beta.json"),
    };
    for (const QString& name : names) {
        QFile file(dir.filePath(name));
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.close();
    }

    SerialProfileCatalogService catalog;

    QCOMPARE(catalog.discoverProfilePaths(tempDir.path()),
             QStringList({dir.absoluteFilePath(QStringLiteral("alpha.edserialprofile")),
                          dir.absoluteFilePath(QStringLiteral("beta.json")),
                          dir.absoluteFilePath(QStringLiteral("zeta.edserialprofile"))}));
}

void SerialProfileCatalogServiceTest::discoverProfilePathsDoesNotModifyCatalog()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QDir dir(tempDir.path());
    QFile file(dir.filePath(QStringLiteral("line-a.edserialprofile")));
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.close();

    SerialProfileCatalogService catalog;
    QVERIFY(!catalog.discoverProfilePaths(tempDir.path()).isEmpty());

    QVERIFY(catalog.recentProfilePaths().isEmpty());
    QCOMPARE(catalog.lastProfilePath(), QString());
}

void SerialProfileCatalogServiceTest::discoverProfilePathsDoesNotScanSubdirectories()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QDir dir(tempDir.path());
    QVERIFY(dir.mkpath(QStringLiteral("nested")));
    QFile nested(dir.filePath(QStringLiteral("nested/line-a.edserialprofile")));
    QVERIFY(nested.open(QIODevice::WriteOnly));
    nested.close();

    SerialProfileCatalogService catalog;

    QVERIFY(catalog.discoverProfilePaths(tempDir.path()).isEmpty());
}

void SerialProfileCatalogServiceTest::importProfileDirectoryRecordsDiscoveredProfiles()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QDir dir(tempDir.path());
    QFile first(dir.filePath(QStringLiteral("alpha.edserialprofile")));
    QVERIFY(first.open(QIODevice::WriteOnly));
    first.close();
    QFile second(dir.filePath(QStringLiteral("beta.edserialprofile")));
    QVERIFY(second.open(QIODevice::WriteOnly));
    second.close();

    SerialProfileCatalogService catalog;

    QCOMPARE(catalog.importProfileDirectory(tempDir.path()), 2);
    QCOMPARE(catalog.recentProfilePaths(),
             QStringList({dir.absoluteFilePath(QStringLiteral("beta.edserialprofile")),
                          dir.absoluteFilePath(QStringLiteral("alpha.edserialprofile"))}));
    QCOMPARE(catalog.lastProfilePath(), dir.absoluteFilePath(QStringLiteral("beta.edserialprofile")));
}

void SerialProfileCatalogServiceTest::importProfileDirectoryReturnsOnlyNewlyImportedCount()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QDir dir(tempDir.path());
    QFile first(dir.filePath(QStringLiteral("alpha.edserialprofile")));
    QVERIFY(first.open(QIODevice::WriteOnly));
    first.close();
    QFile second(dir.filePath(QStringLiteral("beta.edserialprofile")));
    QVERIFY(second.open(QIODevice::WriteOnly));
    second.close();
    SerialProfileCatalogService catalog;
    QVERIFY(catalog.recordProfilePath(dir.absoluteFilePath(QStringLiteral("alpha.edserialprofile"))));

    QCOMPARE(catalog.importProfileDirectory(tempDir.path()), 1);

    QCOMPARE(catalog.recentProfilePaths(),
             QStringList({dir.absoluteFilePath(QStringLiteral("beta.edserialprofile")),
                          dir.absoluteFilePath(QStringLiteral("alpha.edserialprofile"))}));
}

void SerialProfileCatalogServiceTest::importProfileDirectoryKeepsDefaultDirectory()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QDir dir(tempDir.path());
    QFile file(dir.filePath(QStringLiteral("line-a.edserialprofile")));
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.close();
    SerialProfileCatalogService catalog;
    QVERIFY(catalog.setDefaultProfileDirectory(QDir(tempDir.path()).filePath(QStringLiteral("profiles"))));
    const QString before = catalog.defaultProfileDirectory();

    QCOMPARE(catalog.importProfileDirectory(tempDir.path()), 1);

    QCOMPARE(catalog.defaultProfileDirectory(), before);
}

void SerialProfileCatalogServiceTest::importProfileDirectoryRespectsRecentLimit()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QDir dir(tempDir.path());
    for (int i = 0; i < 12; ++i) {
        QFile file(dir.filePath(QStringLiteral("line-%1.edserialprofile").arg(i, 2, 10, QLatin1Char('0'))));
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.close();
    }

    SerialProfileCatalogService catalog;

    QCOMPARE(catalog.importProfileDirectory(tempDir.path()), 12);
    QCOMPARE(catalog.recentProfilePaths().size(), 8);
    QCOMPARE(QFileInfo(catalog.recentProfilePaths().first()).fileName(), QStringLiteral("line-11.edserialprofile"));
    QCOMPARE(QFileInfo(catalog.recentProfilePaths().last()).fileName(), QStringLiteral("line-04.edserialprofile"));
}

void SerialProfileCatalogServiceTest::importProfileDirectoryPersistsAcrossInstances()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QDir dir(tempDir.path());
    QFile file(dir.filePath(QStringLiteral("line-a.edserialprofile")));
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.close();

    {
        SerialProfileCatalogService catalog;
        QCOMPARE(catalog.importProfileDirectory(tempDir.path()), 1);
    }

    SerialProfileCatalogService reloadedCatalog;

    QCOMPARE(reloadedCatalog.recentProfilePaths(),
             QStringList({dir.absoluteFilePath(QStringLiteral("line-a.edserialprofile"))}));
}

void SerialProfileCatalogServiceTest::importProfileDirectoryHandlesJsonProfiles()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QDir dir(tempDir.path());
    QFile file(dir.filePath(QStringLiteral("legacy.json")));
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.close();

    SerialProfileCatalogService catalog;

    QCOMPARE(catalog.importProfileDirectory(tempDir.path()), 1);
    QCOMPARE(catalog.recentProfilePaths(), QStringList({dir.absoluteFilePath(QStringLiteral("legacy.json"))}));
}

void SerialProfileCatalogServiceTest::discoverProfilePathsTrimsDirectoryInput()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QDir dir(tempDir.path());
    QFile file(dir.filePath(QStringLiteral("line-a.edserialprofile")));
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.close();

    SerialProfileCatalogService catalog;

    QCOMPARE(catalog.discoverProfilePaths(QStringLiteral("  %1  ").arg(tempDir.path())),
             QStringList({dir.absoluteFilePath(QStringLiteral("line-a.edserialprofile"))}));
}

void SerialProfileCatalogServiceTest::discoverProfilePathsKeepsCatalogDefaultDirectory()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QDir dir(tempDir.path());
    QFile file(dir.filePath(QStringLiteral("line-a.edserialprofile")));
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.close();
    SerialProfileCatalogService catalog;
    QVERIFY(catalog.setDefaultProfileDirectory(QDir(tempDir.path()).filePath(QStringLiteral("profiles"))));
    const QString before = catalog.defaultProfileDirectory();

    QVERIFY(!catalog.discoverProfilePaths(tempDir.path()).isEmpty());

    QCOMPARE(catalog.defaultProfileDirectory(), before);
}

void SerialProfileCatalogServiceTest::importMissingProfileDirectoryReturnsZero()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    SerialProfileCatalogService catalog;

    QCOMPARE(catalog.importProfileDirectory(QDir(tempDir.path()).filePath(QStringLiteral("missing"))), 0);

    QVERIFY(catalog.recentProfilePaths().isEmpty());
    QCOMPARE(catalog.lastProfilePath(), QString());
}

void SerialProfileCatalogServiceTest::importEmptyProfileDirectoryKeepsExistingCatalog()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    SerialProfileCatalogService catalog;
    QVERIFY(catalog.recordProfilePath(QStringLiteral("C:/profiles/existing.edserialprofile")));
    const QStringList beforeRecent = catalog.recentProfilePaths();
    const QString beforeLast = catalog.lastProfilePath();

    QCOMPARE(catalog.importProfileDirectory(tempDir.path()), 0);

    QCOMPARE(catalog.recentProfilePaths(), beforeRecent);
    QCOMPARE(catalog.lastProfilePath(), beforeLast);
}

void SerialProfileCatalogServiceTest::importProfileDirectoryKeepsUnrelatedRecentProfiles()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QDir dir(tempDir.path());
    QFile file(dir.filePath(QStringLiteral("line-a.edserialprofile")));
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.close();
    SerialProfileCatalogService catalog;
    QVERIFY(catalog.recordProfilePath(QStringLiteral("C:/profiles/existing.edserialprofile")));

    QCOMPARE(catalog.importProfileDirectory(tempDir.path()), 1);

    QCOMPARE(catalog.recentProfilePaths(),
             QStringList({dir.absoluteFilePath(QStringLiteral("line-a.edserialprofile")),
                          QStringLiteral("C:/profiles/existing.edserialprofile")}));
}

QTEST_GUILESS_MAIN(SerialProfileCatalogServiceTest)
#include "test_serial_profile_catalog_service.moc"
