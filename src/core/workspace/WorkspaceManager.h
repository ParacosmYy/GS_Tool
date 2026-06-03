// Copyright 2024 EmbedDebug Project
// SPDX-License-Identifier: MIT
#pragma once
#include <QObject>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QVariantMap>

struct WorkspaceLayout {
    QString name;
    QVariantMap panelStates;
    QStringList visiblePanels;
    QString mainWindowState;
    QString geometry;
};

class WorkspaceManager : public QObject {
    Q_OBJECT
public:
    explicit WorkspaceManager(QObject *parent = nullptr);
    ~WorkspaceManager() override;

    void saveWorkspace(const WorkspaceLayout &layout);
    WorkspaceLayout loadWorkspace(const QString &name) const;
    void deleteWorkspace(const QString &name);
    QStringList workspaceNames() const;
    bool exists(const QString &name) const;
    void setActiveWorkspace(const QString &name);
    QString activeWorkspace() const;

    void exportToFile(const QString &name, const QString &filePath) const;
    bool importFromFile(const QString &filePath);

signals:
    void workspaceSaved(const QString &name);
    void workspaceLoaded(const QString &name);
    void workspaceDeleted(const QString &name);
    void activeWorkspaceChanged(const QString &name);

private:
    QMap<QString, WorkspaceLayout> m_workspaces;
    QString m_activeWorkspace;
};