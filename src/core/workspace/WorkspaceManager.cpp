/**
 * @file WorkspaceManager.cpp
 * @brief 工作区管理器实现 — 工作区布局的增删改查、导入导出、激活切换
 */
#include "core/workspace/WorkspaceManager.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

namespace {

QString normalizedWorkspaceName(const QString &name)
{
    return name.trimmed();
}

}  // namespace

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
    const QString name = normalizedWorkspaceName(layout.name);
    if (name.isEmpty()) {
        return;
    }

    WorkspaceLayout normalizedLayout = layout;
    normalizedLayout.name = name;

    // 首次保存(新建)时递增创建计数，否则递增覆盖保存计数
    if (!m_workspaces.contains(name)) {
        ++m_totalWorkspacesCreated;
    } else {
        ++m_totalOverwriteSaves;
    }
    ++m_totalSaves;
    m_workspaces[name] = normalizedLayout;
    emit workspaceSaved(name);
}

/** @brief 按名称加载工作区布局 @param name 工作区名称 @return 工作区布局，不存在时返回默认构造 */
WorkspaceLayout WorkspaceManager::loadWorkspace(const QString &name) const
{
    const QString normalizedName = normalizedWorkspaceName(name);
    ++m_totalLoads;
    if (!m_workspaces.contains(normalizedName)) {
        ++m_totalLoadMisses;  ///< 累计加载不存在工作区次数
    }
    return m_workspaces.value(normalizedName);
}

/** @brief 删除指定工作区，若为当前激活则清空激活状态 @param name 工作区名称 */
void WorkspaceManager::deleteWorkspace(const QString &name)
{
    const QString normalizedName = normalizedWorkspaceName(name);
    if (normalizedName.isEmpty() || !m_workspaces.contains(normalizedName)) {
        return;
    }

    ++m_totalDeletions;
    ++m_totalWorkspacesDeleted;
    m_workspaces.remove(normalizedName);
    const bool wasActive = (m_activeWorkspace == normalizedName);
    if (wasActive) {
        m_activeWorkspace.clear();
    }
    emit workspaceDeleted(normalizedName);
    if (wasActive) {
        emit activeWorkspaceChanged(QString());
    }
}

/** @brief 获取所有已保存的工作区名称 @return 名称列表 */
QStringList WorkspaceManager::workspaceNames() const { return m_workspaces.keys(); }
/** @brief 检查指定名称的工作区是否存在 @param name 工作区名称 @return 存在返回true */
bool WorkspaceManager::exists(const QString &name) const
{
    return m_workspaces.contains(normalizedWorkspaceName(name));
}

/** @brief 设置当前激活工作区，切换时累计前一个工作区的活跃时长 @param name 工作区名称 */
void WorkspaceManager::setActiveWorkspace(const QString &name)
{
    const QString normalizedName = normalizedWorkspaceName(name);
    if (!normalizedName.isEmpty() && !m_workspaces.contains(normalizedName)) {
        return;
    }

    if (m_activeWorkspace != normalizedName) {
        // 累计前一个工作区的活跃时长
        if (m_activeTimer.isValid()) {
            m_activeWorkspaceTimeMs += static_cast<quint64>(m_activeTimer.elapsed());
        }
        ++m_totalSwitches;
        m_activeWorkspace = normalizedName;
        m_activeTimer.restart();
        emit activeWorkspaceChanged(normalizedName);
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

/** @brief 重置所有工作区统计计数器(保存/加载/删除/切换/新建/活跃时长/导出错误/导入错误/加载未命中/覆盖保存) */
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
    m_totalExportErrors = 0;
    m_totalImportErrors = 0;
    m_totalLoadMisses = 0;
    m_totalOverwriteSaves = 0;
    m_activeTimer.restart();
}

/** @brief 将工作区布局导出为JSON文件 @param name 工作区名称 @param filePath 导出文件路径 */
void WorkspaceManager::exportToFile(const QString &name, const QString &filePath) const
{
    const QString normalizedName = normalizedWorkspaceName(name);
    const QString normalizedPath = filePath.trimmed();

    ++m_totalExportFiles;
    if (normalizedPath.isEmpty()) {
        ++m_totalExportErrors;  // 导出失败: 文件路径为空
        return;
    }

    auto it = m_workspaces.constFind(normalizedName);
    if (it == m_workspaces.constEnd()) {
        ++m_totalExportErrors;  // 导出失败: 工作区不存在
        return;
    }
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
    QFile f(normalizedPath);
    if (!f.open(QIODevice::WriteOnly)) {
        ++m_totalExportErrors;  // 导出失败: 文件写入错误
        return;
    }
    f.write(QJsonDocument(root).toJson());
}

/** @brief 从JSON文件导入工作区布局 @param filePath 导入文件路径 @return 导入成功返回true */
bool WorkspaceManager::importFromFile(const QString &filePath)
{
    const QString normalizedPath = filePath.trimmed();
    if (normalizedPath.isEmpty()) {
        ++m_totalImportErrors;  // 导入失败: 文件路径为空
        return false;
    }

    QFile f(normalizedPath);
    if (!f.open(QIODevice::ReadOnly)) {
        ++m_totalImportErrors;  // 导入失败: 文件读取错误
        return false;
    }

    const auto doc = QJsonDocument::fromJson(f.readAll());
    if (!doc.isObject()) {
        ++m_totalImportErrors;  // 导入失败: JSON格式错误
        return false;
    }
    auto obj = doc.object();
    WorkspaceLayout layout;
    layout.name = normalizedWorkspaceName(obj["name"].toString());
    if (layout.name.isEmpty()) {
        ++m_totalImportErrors;  // 导入失败: 工作区名称为空
        return false;
    }

    layout.mainWindowState = obj["mainWindowState"].toString();
    layout.geometry = obj["geometry"].toString();
    for (const auto &v : obj["visiblePanels"].toArray()) layout.visiblePanels << v.toString();
    auto states = obj["panelStates"].toObject();
    for (auto it = states.constBegin(); it != states.constEnd(); ++it)
        layout.panelStates[it.key()] = it.value().toVariant();
    saveWorkspace(layout);
    ++m_totalImportFiles;
    return true;
}
