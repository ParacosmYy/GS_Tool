/**
 * @file PluginManagerScan.cpp
 * @brief 插件扫描发现与统计查询实现
 *
 * 从 PluginManager 拆分而来，集中管理插件扫描（scanPlugins）、
 * 查询接口（plugin/pluginVersion/pluginDescription 等）以及
 * 运行时统计数据（加载/失败/卸载次数、重置统计）。
 *
 * @see PluginManager.h 头文件中完整的接口声明
 */

#include "plugin/PluginManager.h"
#include "plugin/PluginApi.h"

#include <QDir>
#include <QLibrary>

/**
 * @brief createPlugin 导出函数签名
 *
 * 每个插件 DLL 必须导出此符号:
 * @code
 * extern "C" IEmbedDebugPlugin* createPlugin() {
 *     return new MyPlugin();
 * }
 * @endcode
 */
using CreatePluginFunc = IEmbedDebugPlugin* (*)();

/**
 * @brief 扫描目录中的插件文件
 *
 * 遍历指定目录，查找所有 DLL（Windows）或 SO（Linux）文件，
 * 尝试解析 createPlugin 导出符号以判断是否为合法插件。
 *
 * @param pluginDir 插件目录路径
 * @return 找到的插件候选文件路径列表
 */
QStringList PluginManager::scanPlugins(const QString& pluginDir)
{
    QStringList candidates;
    QDir dir(pluginDir);
    if (!dir.exists()) {
        emit pluginError(pluginDir, tr("插件目录不存在: %1").arg(pluginDir));
        return candidates;
    }

    ++m_scanCount;

    /* 平台扩展名过滤 */
#ifdef Q_OS_WIN
    const QString filter = QStringLiteral("*.dll");
#else
    const QString filter = QStringLiteral("*.so");
#endif

    const QFileInfoList entries = dir.entryInfoList(
        QStringList() << filter, QDir::Files, QDir::Name);

    for (const QFileInfo& fi : entries) {
        const QString absPath = fi.absoluteFilePath();
        QLibrary lib(absPath);
        if (!lib.load()) {
            continue;
        }
        auto createFn = reinterpret_cast<CreatePluginFunc>(
            lib.resolve("createPlugin"));
        lib.unload();

        if (createFn) {
            candidates.append(absPath);
        }
    }

    m_discoveredFiles += candidates.size();
    return candidates;
}

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

/**
 * @brief 获取累计加载成功次数
 */
quint64 PluginManager::totalLoadCount() const
{
    return m_loadCount;
}

/**
 * @brief 获取累计加载失败次数
 */
quint64 PluginManager::totalFailCount() const
{
    return m_failCount;
}

/**
 * @brief 获取累计卸载次数
 */
quint64 PluginManager::totalUnloadCount() const
{
    return m_unloadCount;
}

/**
 * @brief 重置所有插件管理统计(加载/失败/卸载/扫描/发现文件)
 */
void PluginManager::resetLoadStatistics()
{
    m_loadCount = 0;
    m_failCount = 0;
    m_unloadCount = 0;
    m_scanCount = 0;
    m_discoveredFiles = 0;
}
