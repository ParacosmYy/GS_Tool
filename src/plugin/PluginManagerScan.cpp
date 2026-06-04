/**
 * @file PluginManagerScan.cpp
 * @brief 插件扫描发现与统计查询实现
 *
 * 从 PluginManager 拆分而来，集中管理插件扫描（scanPlugins）和
 * 运行时统计数据（加载/失败/卸载次数、重置统计）。
 *
 * 查询接口(plugin/pluginVersion/pluginDescription等)见 PluginManagerQuery.cpp。
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

/** @brief 重置所有插件管理统计计数器(含加载/卸载/扫描/错误/发现文件) */
void PluginManager::resetStats()
{
    resetLoadStatistics();
}

// 查询接口(plugin/pluginVersion/pluginDescription等)见 PluginManagerQuery.cpp
