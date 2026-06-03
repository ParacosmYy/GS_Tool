#include "core/workspace/WorkspaceManager.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

WorkspaceManager::WorkspaceManager(QObject *parent) : QObject(parent) {}
WorkspaceManager::~WorkspaceManager() = default;

void WorkspaceManager::saveWorkspace(const WorkspaceLayout &layout)
{
    m_workspaces[layout.name] = layout;
    emit workspaceSaved(layout.name);
}

WorkspaceLayout WorkspaceManager::loadWorkspace(const QString &name) const
{
    return m_workspaces.value(name);
}

void WorkspaceManager::deleteWorkspace(const QString &name)
{
    m_workspaces.remove(name);
    if (m_activeWorkspace == name) m_activeWorkspace.clear();
    emit workspaceDeleted(name);
}

QStringList WorkspaceManager::workspaceNames() const { return m_workspaces.keys(); }
bool WorkspaceManager::exists(const QString &name) const { return m_workspaces.contains(name); }

void WorkspaceManager::setActiveWorkspace(const QString &name)
{
    if (m_activeWorkspace != name) {
        m_activeWorkspace = name;
        emit activeWorkspaceChanged(name);
    }
}

QString WorkspaceManager::activeWorkspace() const { return m_activeWorkspace; }

void WorkspaceManager::exportToFile(const QString &name, const QString &filePath) const
{
    auto it = m_workspaces.constFind(name);
    if (it == m_workspaces.constEnd()) return;
    const auto &layout = it.value();
    QJsonObject root;
    root["name"] = layout.name;
    root["mainWindowState"] = layout.mainWindowState;
    root["geometry"] = layout.geometry;
    QJsonArray panels;
    for (const auto &p : layout.visiblePanels) panels.append(p);
    root["visiblePanels"] = panels;
    QJsonObject states;
    for (auto it2 = layout.panelStates.constBegin(); it2 != layout.panelStates.constEnd(); ++it2)
        states[it2.key()] = QJsonValue::fromVariant(it2.value());
    root["panelStates"] = states;
    QFile f(filePath);
    if (f.open(QIODevice::WriteOnly)) f.write(QJsonDocument(root).toJson());
}

bool WorkspaceManager::importFromFile(const QString &filePath)
{
    QFile f(filePath);
    if (!f.open(QIODevice::ReadOnly)) return false;
    auto doc = QJsonDocument::fromJson(f.readAll());
    if (!doc.isObject()) return false;
    auto obj = doc.object();
    WorkspaceLayout layout;
    layout.name = obj["name"].toString();
    layout.mainWindowState = obj["mainWindowState"].toString();
    layout.geometry = obj["geometry"].toString();
    for (const auto &v : obj["visiblePanels"].toArray()) layout.visiblePanels << v.toString();
    auto states = obj["panelStates"].toObject();
    for (auto it = states.constBegin(); it != states.constEnd(); ++it)
        layout.panelStates[it.key()] = it.value().toVariant();
    saveWorkspace(layout);
    return true;
}