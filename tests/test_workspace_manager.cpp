#include <QtCore/QFile>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QTemporaryDir>
#include <QtTest/QSignalSpy>
#include <QtTest/QtTest>

#include "core/workspace/WorkspaceManager.h"

class TestWorkspaceManager : public QObject {
    Q_OBJECT

private slots:
    void saveWorkspaceRejectsBlankName()
    {
        WorkspaceManager manager;
        QSignalSpy savedSpy(&manager, &WorkspaceManager::workspaceSaved);

        WorkspaceLayout layout;
        layout.name = QStringLiteral("   ");
        layout.visiblePanels = {QStringLiteral("terminal")};

        manager.saveWorkspace(layout);

        QVERIFY(manager.workspaceNames().isEmpty());
        QCOMPARE(manager.totalSaves(), quint64(0));
        QCOMPARE(manager.totalWorkspacesCreated(), quint64(0));
        QCOMPARE(savedSpy.count(), 0);
    }

    void saveWorkspaceTrimsNameForLookupAndSignals()
    {
        WorkspaceManager manager;
        QSignalSpy savedSpy(&manager, &WorkspaceManager::workspaceSaved);

        WorkspaceLayout layout;
        layout.name = QStringLiteral("  调试布局  ");
        layout.visiblePanels = {QStringLiteral("terminal")};

        manager.saveWorkspace(layout);

        QVERIFY(manager.exists(QStringLiteral("调试布局")));
        QVERIFY(manager.exists(QStringLiteral("  调试布局  ")));
        QCOMPARE(manager.workspaceNames(), QStringList{QStringLiteral("调试布局")});
        QCOMPARE(manager.loadWorkspace(QStringLiteral("调试布局")).name, QStringLiteral("调试布局"));
        QCOMPARE(savedSpy.count(), 1);
        QCOMPARE(savedSpy.takeFirst().at(0).toString(), QStringLiteral("调试布局"));
    }

    void setActiveWorkspaceIgnoresMissingWorkspace()
    {
        WorkspaceManager manager;
        QSignalSpy activeSpy(&manager, &WorkspaceManager::activeWorkspaceChanged);

        manager.setActiveWorkspace(QStringLiteral("missing"));

        QVERIFY(manager.activeWorkspace().isEmpty());
        QCOMPARE(manager.totalSwitches(), quint64(0));
        QCOMPARE(activeSpy.count(), 0);
    }

    void setActiveWorkspaceAcceptsTrimmedExistingName()
    {
        WorkspaceManager manager;
        QSignalSpy activeSpy(&manager, &WorkspaceManager::activeWorkspaceChanged);

        WorkspaceLayout layout;
        layout.name = QStringLiteral("默认布局");
        manager.saveWorkspace(layout);

        manager.setActiveWorkspace(QStringLiteral("  默认布局  "));

        QCOMPARE(manager.activeWorkspace(), QStringLiteral("默认布局"));
        QCOMPARE(manager.totalSwitches(), quint64(1));
        QCOMPARE(activeSpy.count(), 1);
        QCOMPARE(activeSpy.takeFirst().at(0).toString(), QStringLiteral("默认布局"));
    }

    void deletingActiveWorkspaceEmitsClearActiveSignal()
    {
        WorkspaceManager manager;

        WorkspaceLayout layout;
        layout.name = QStringLiteral("默认布局");
        manager.saveWorkspace(layout);
        manager.setActiveWorkspace(QStringLiteral("默认布局"));

        QSignalSpy activeSpy(&manager, &WorkspaceManager::activeWorkspaceChanged);
        QSignalSpy deletedSpy(&manager, &WorkspaceManager::workspaceDeleted);

        manager.deleteWorkspace(QStringLiteral("默认布局"));

        QVERIFY(manager.activeWorkspace().isEmpty());
        QCOMPARE(deletedSpy.count(), 1);
        QCOMPARE(activeSpy.count(), 1);
        QCOMPARE(activeSpy.takeFirst().at(0).toString(), QString());
    }

    void importFromFileRejectsMissingName()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString path = dir.filePath(QStringLiteral("workspace.json"));

        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write(QJsonDocument(QJsonObject{{QStringLiteral("visiblePanels"), QJsonArray{QStringLiteral("terminal")}}}).toJson());
        file.close();

        WorkspaceManager manager;

        QVERIFY(!manager.importFromFile(path));
        QVERIFY(manager.workspaceNames().isEmpty());
        QCOMPARE(manager.totalImportErrors(), quint64(1));
        QCOMPARE(manager.totalSaves(), quint64(0));
    }

    void exportToFileRejectsBlankPathWithoutCreatingFile()
    {
        WorkspaceManager manager;
        WorkspaceLayout layout;
        layout.name = QStringLiteral("默认布局");
        manager.saveWorkspace(layout);

        manager.exportToFile(QStringLiteral("默认布局"), QStringLiteral("   "));

        QCOMPARE(manager.totalExportFiles(), quint64(1));
        QCOMPARE(manager.totalExportErrors(), quint64(1));
    }
};

QTEST_MAIN(TestWorkspaceManager)
#include "test_workspace_manager.moc"
