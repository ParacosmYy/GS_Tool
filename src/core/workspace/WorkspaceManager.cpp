/**
 * @file WorkspaceManager.cpp
 * @brief 工作区管理器实现 — 工作区布局的增删改查、导入导出、激活切换
 */
#include "core/workspace/WorkspaceManager.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

/** @brief 构造函数 @param parent 父对象 */
WorkspaceManager::WorkspaceManager(QObject *parent) : QObject(parent)
{
    m_activeTimer.start();
}
/** @brief 析构函数 */
WorkspaceManager::~WorkspaceManager() = default;

/** @brief 保存工作区布局到内存映射，新建工作区时递增创建计数 @param layout 工作区布局配置 */
void WorkspaceManager::saveWorkspace(const WorkspaceLayout &layout)
{
    // 首次保存(新建)时递增创建计数
    if (!m_workspaces.contains(layout.name)) {
        ++m_totalWorkspacesCreated;
    }
    ++m_totalSaves;
    m_workspaces[layout.name] = layout;
    emit workspaceSaved(layout.name);
}

/** @brief 按名称加载工作区布局 @param name 工作区名称 @return 工作区布局，不存在时返回默认构造 */
WorkspaceLayout WorkspaceManager::loadWorkspace(const QString &name) const
{
    ++m_totalLoads;
    return m_workspaces.value(name);
}

/** @brief 删除指定工作区，若为当前激活则清空激活状态 @param name 工作区名称 */
void WorkspaceManager::deleteWorkspace(const QString &name)
{
    ++m_totalDeletions;
    ++m_totalWorkspacesDeleted;
    m_workspaces.remove(name);
    if (m_activeWorkspace == name) m_activeWorkspace.clear();
    emit workspaceDeleted(name);
}

/** @brief 获取所有已保存的工作区名称 @return 名称列表 */
QStringList WorkspaceManager::workspaceNames() const { return m_workspaces.keys(); }
/** @brief 检查指定名称的工作区是否存在 @param name 工作区名称 @return 存在返回true */
bool WorkspaceManager::exists(const QString &name) const { return m_workspaces.contains(name); }

/** @brief 设置当前激活工作区，切换时累计前一个工作区的活跃时长 @param name 工作区名称 */
void WorkspaceManager::setActiveWorkspace(const QString &name)
{
    if (m_activeWorkspace != name) {
        // 累计前一个工作区的活跃时长
        if (m_activeTimer.isValid()) {
            m_activeWorkspaceTimeMs += static_cast<quint64>(m_activeTimer.elapsed());
        }
        ++m_totalSwitches;
        m_activeWorkspace = name;
        m_activeTimer.restart();
        emit activeWorkspaceChanged(name);
    }
}

/** @brief 获取当前激活的工作区名称 @return 工作区名称 */
QString WorkspaceManager::activeWorkspace() const { return m_activeWorkspace; }

/** @brief 获取当前激活工作区累计活跃时长(含正在进行的计时) @return 活跃时长毫秒数 */
quint64 WorkspaceManager::activeWorkspaceTimeMs() const
{
    quint64 total = m_activeWorkspaceTimeMs;
    if (m_activeTimer.isValid()) {
        total += static_cast<quint64>(m_activeTimer.elapsed());
    }
    return total;
}

/** @brief 重置所有工作区统计计数器(保存/加载/删除/切换/新建/活跃时长) */
void WorkspaceManager::resetWorkspaceStatistics()
{
    m_totalSaves = 0;
    m_totalLoads = 0;
    m_totalDeletions = 0;
    m_totalSwitches = 0;
    m_totalExportFiles = 0;
    m_totalImportFiles = 0;
    m_totalWorkspacesCreated = 0;
    m_totalWorkspacesDeleted = 0;
    m_activeWorkspaceTimeMs = 0;
    m_activeTimer.restart();
}

/** @brief 将工作区布局导出为JSON文件 @param name 工作区名称 @param filePath 导出文件路径 */
void WorkspaceManager::exportToFile(const QString &name, const QString &filePath) const
{
    ++m_totalExportFiles;
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

/** @brief 从JSON文件导入工作区布局 @param filePath 导入文件路径 @return 导入成功返回true */
bool WorkspaceManager::importFromFile(const QString &filePath)
{
    QFile f(filePath);
    if (!f.open(QIODevice::ReadOnly)) return false;
    ++m_totalImportFiles;
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
