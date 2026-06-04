/**
 * @file PluginLoader2.h
 * @brief 第二版插件加载器，支持目录扫描和更精简的插件管理接口
 */
// Copyright 2024 EmbedDebug Project
// SPDX-License-Identifier: MIT
#pragma once
#include <QObject>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QPluginLoader>

/**
 * @brief 插件加载器v2统计计数器
 *
 * 追踪插件加载尝试、成功/失败、卸载、扫描和发现指标。
 */
struct PluginLoader2Stats {
    quint64 totalPluginsDiscovered = 0;   ///< 累计发现插件文件次数
    quint64 totalLoadAttempts = 0;        ///< 总加载尝试次数
    quint64 totalLoadSuccesses = 0;       ///< 总加载成功次数
    quint64 totalLoadFailures = 0;        ///< 总加载失败次数
    quint64 totalUnloads = 0;             ///< 总卸载次数
    quint64 totalScanRuns = 0;            ///< 总扫描次数
    quint64 totalDirectoriesScanned = 0;  ///< 累计扫描目录数
};

/**
 * @class PluginLoader
 * @brief 第二版插件加载器，提供搜索路径管理、目录扫描和按名称加载/卸载插件的能力
 */
class PluginLoader : public QObject {
    Q_OBJECT
public:
    /** @brief 构造函数 @param parent 父对象指针 */
    explicit PluginLoader(QObject *parent = nullptr);
    /** @brief 析构函数 */
    ~PluginLoader() override;

    /** @brief 添加插件搜索路径 @param path 目录路径 */
    void addSearchPath(const QString &path);
    /** @brief 移除插件搜索路径 @param path 目录路径 */
    void removeSearchPath(const QString &path);
    /** @brief 获取所有搜索路径 @return 路径列表 */
    QStringList searchPaths() const;

    /** @brief 加载指定路径的插件 @param filePath 插件文件路径 @return 是否加载成功 */
    bool loadPlugin(const QString &filePath);
    /** @brief 卸载指定名称的插件 @param name 插件名称 */
    void unloadPlugin(const QString &name);
    /** @brief 卸载所有已加载插件 */
    void unloadAll();

    /** @brief 获取已加载插件的实例对象 @param name 插件名称 @return 插件实例指针，未找到返回nullptr */
    QObject* pluginInstance(const QString &name) const;
    /** @brief 获取所有已加载插件名称列表 @return 名称列表 */
    QStringList loadedPlugins() const;
    /** @brief 获取所有已扫描但未加载的插件名称列表 @return 名称列表 */
    QStringList discoveredPlugins() const;
    /** @brief 查询插件是否已加载 @param name 插件名称 @return 是否已加载 */
    bool isLoaded(const QString &name) const;
    /** @brief 查询插件是否已被扫描发现(未加载) @param name 插件名称 @return 是否已发现 */
    bool isDiscovered(const QString &name) const;
    /** @brief 获取已发现插件的文件路径 @param name 插件名称 @return 文件路径，未找到返回空 */
    QString discoveredPluginPath(const QString &name) const;

    /** @brief 扫描指定目录下的所有插件文件并尝试加载 @param path 目录路径 */
    void scanDirectory(const QString &path);

    // ---- Stats struct 接口 ----

    /** @brief 获取统计计数器只读引用 @return 当前统计快照 */
    const PluginLoader2Stats& stats() const { return m_stats; }

    /** @brief 重置所有统计计数器为初始值 */
    void resetStats() { m_stats = PluginLoader2Stats{}; }

    // ---- 兼容性 getter（委托给 m_stats） ----

    /** @brief 获取总加载尝试次数 @return 累计尝试次数 */
    quint64 totalLoadAttempts() const { return m_stats.totalLoadAttempts; }
    /** @brief 获取总加载成功次数 @return 累计成功次数 */
    quint64 totalLoadSuccesses() const { return m_stats.totalLoadSuccesses; }
    /** @brief 获取总卸载次数 @return 累计卸载次数 */
    quint64 totalUnloads() const { return m_stats.totalUnloads; }
    /** @brief 获取总扫描次数 @return 累计扫描次数 */
    quint64 totalScanRuns() const { return m_stats.totalScanRuns; }
    /** @brief 重置加载器统计（兼容旧接口） */
    void resetLoaderStatistics() { resetStats(); }

signals:
    /** @brief 插件加载完成时发射 @param name 插件名称 @param filePath 文件路径 */
    void pluginLoaded(const QString &name, const QString &filePath);
    /** @brief 插件卸载完成时发射 @param name 插件名称 */
    void pluginUnloaded(const QString &name);
    /** @brief 插件操作出错时发射 @param name 插件名称 @param error 错误信息 */
    void pluginError(const QString &name, const QString &error);
    /** @brief 扫描发现新插件时发射 @param name 插件名称 @param filePath 文件路径 */
    void pluginFound(const QString &name, const QString &filePath);

private:
    /**
     * @brief 插件条目，保存加载器、实例和文件路径
     */
    struct PluginEntry {
        QString name;                          ///< 插件名称
        QString filePath;                      ///< 插件文件路径
        QPluginLoader *loader = nullptr;       ///< Qt插件加载器
        QObject *instance = nullptr;           ///< 插件实例指针
    };

    QMap<QString, PluginEntry> m_plugins;  ///< 插件名称到条目的映射(已加载)
    QMap<QString, PluginEntry> m_discoveredPlugins; ///< 已扫描发现但未加载的插件
    QStringList m_searchPaths;              ///< 插件搜索路径列表

    PluginLoader2Stats m_stats;            ///< 加载器v2统计计数器
};
