/**
 * @file PluginManagerQuery.cpp
 * @brief 插件管理器查询与元数据接口实现
 *
 * 从 PluginManagerScan.cpp 拆分而来，集中管理插件的查询接口:
 *   - loadedPluginNames/plugin/pluginVersion/pluginDescription
 *   - pluginCount/isPluginLoaded/pluginMetadataList
 *
 * 插件扫描发现见 PluginManagerScan.cpp。
 */

#include "plugin/PluginManager.h"
#include "plugin/PluginApi.h"

/**
 * @brief 获取所有已加载插件的名称列表
 * @return 插件名称列表
 */
QStringList PluginManager::loadedPluginNames() const
{
    return m_plugins.keys();
}

/**
 * @brief 获取指定插件的接口指针
 * @param name 插件名称
 * @return 插件接口指针，未找到返回 nullptr
 */
IEmbedDebugPlugin* PluginManager::plugin(const QString& name) const
{
    return m_plugins.value(name, nullptr);
}

/**
 * @brief 获取指定插件的版本号
 * @param name 插件名称
 * @return 版本字符串，未找到返回空字符串
 */
QString PluginManager::pluginVersion(const QString& name) const
{
    IEmbedDebugPlugin* plug = m_plugins.value(name, nullptr);
    return plug ? plug->version() : QString();
}

/**
 * @brief 获取指定插件的描述信息
 * @param name 插件名称
 * @return 描述字符串，未找到返回空字符串
 */
QString PluginManager::pluginDescription(const QString& name) const
{
    IEmbedDebugPlugin* plug = m_plugins.value(name, nullptr);
    return plug ? plug->description() : QString();
}

/**
 * @brief 获取已加载插件数量
 * @return 插件数量
 */
int PluginManager::pluginCount() const
{
    return m_plugins.count();
}

/**
 * @brief 检查指定插件是否已加载
 * @param name 插件名称
 * @return true 已加载，false 未加载
 */
bool PluginManager::isPluginLoaded(const QString& name) const
{
    return m_plugins.contains(name);
}

/**
 * @brief 获取所有插件的综合元数据列表
 *
 * 返回 QVariantList，每个元素为 QVariantMap，包含:
 *   - "name": 插件名称
 *   - "version": 版本号
 *   - "description": 插件描述
 *
 * @return 插件元数据列表
 */
QVariantList PluginManager::pluginMetadataList() const
{
    QVariantList result;
    for (auto it = m_plugins.constBegin(); it != m_plugins.constEnd(); ++it) {
        QVariantMap meta;
        meta[QStringLiteral("name")] = it.key();
        meta[QStringLiteral("version")] = it.value()->version();
        meta[QStringLiteral("description")] = it.value()->description();
        result.append(meta);
    }
    return result;
}
