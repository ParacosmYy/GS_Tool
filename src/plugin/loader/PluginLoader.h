/**
 * @file PluginLoader.h
 * @brief 插件加载器 — 基于Qt插件系统的动态加载/卸载/目录扫描
 */
#pragma once
#include <QObject>
#include <QMap>
#include <QString>
#include <QPluginLoader>
#include <QList>

/**
 * @brief 插件加载器统计计数器
 *
 * 追踪插件发现、加载/卸载、扫描和失败指标。
 */
struct PluginLoaderStats {
    quint64 totalPluginsDiscovered = 0;   ///< 累计发现插件文件次数
    quint64 totalLoadSuccesses = 0;       ///< 累计加载成功次数
    quint64 totalLoadFailures = 0;        ///< 累计加载失败次数
    quint64 totalUnloads = 0;             ///< 累计卸载次数
    quint64 totalScans = 0;               ///< 累计扫描次数
    quint64 totalDirectoriesScanned = 0;  ///< 累计扫描目录数
};

/**
 * @class PluginLoader
 * @brief 插件加载器，管理插件搜索路径、文件扫描和动态加载/卸载
 */
class PluginLoader : public QObject {
    Q_OBJECT
public:
    /** @brief 插件信息结构 */
    struct PluginInfo {
        QString name;             ///< 插件名称
        QString version;          ///< 插件版本
        QString description;      ///< 插件描述
        QString filePath;         ///< 插件文件路径
        bool loaded = false;      ///< 是否已加载
    };

    /** @brief 构造函数 @param parent 父对象指针 */
    explicit PluginLoader(QObject *parent = nullptr);
    /** @brief 析构函数，自动卸载所有插件 */
    ~PluginLoader() override;

    /** @brief 添加插件搜索路径 @param path 目录路径 */
    void addSearchPath(const QString &path);
    /** @brief 移除插件搜索路径 @param path 目录路径 */
    void removeSearchPath(const QString &path);
    /** @brief 获取所有搜索路径 @return 路径列表 */
    QStringList searchPaths() const;

    /** @brief 加载指定路径的插件文件 @param filePath 插件文件路径 @return 加载成功返回true */
    bool loadPlugin(const QString &filePath);
    /** @brief 卸载指定名称的插件 @param name 插件名称 */
    void unloadPlugin(const QString &name);
    /** @brief 卸载所有已加载插件 */
    void unloadAll();

    /** @brief 获取插件实例对象 @param name 插件名称 @return QObject指针，不存在返回nullptr */
    QObject* pluginInstance(const QString &name) const;
    /** @brief 获取所有已加载插件信息 @return 插件信息列表 */
    QList<PluginInfo> loadedPlugins() const;
    /** @brief 扫描所有搜索路径中的插件 @return 发现的插件信息列表 */
    QList<PluginInfo> scanPlugins();

    // ---- Stats struct 接口 ----

    /** @brief 获取统计计数器只读引用 @return 当前统计快照 */
    const PluginLoaderStats& stats() const { return m_stats; }

    /** @brief 重置所有统计计数器为初始值 */
    void resetStats() { m_stats = PluginLoaderStats{}; }

    // ---- 兼容性 getter（委托给 m_stats） ----

    /** @brief 获取累计加载成功次数 @return 成功次数 */
    quint64 totalLoadSuccesses() const { return m_stats.totalLoadSuccesses; }
    /** @brief 获取累计加载失败次数 @return 失败次数 */
    quint64 totalLoadFailures() const { return m_stats.totalLoadFailures; }
    /** @brief 获取累计卸载次数 @return 卸载次数 */
    quint64 totalUnloads() const { return m_stats.totalUnloads; }
    /** @brief 获取累计扫描次数 @return 扫描次数 */
    quint64 totalScans() const { return m_stats.totalScans; }
    /** @brief 重置所有统计计数器（兼容旧接口） */
    void resetLoaderStatistics() { resetStats(); }

signals:
    /** @brief 扫描发现新插件 @param info 插件信息 */
    void pluginFound(const PluginInfo &info);
    /** @brief 插件加载完成 @param name 插件名称 */
    void pluginLoaded(const QString &name);
    /** @brief 插件卸载完成 @param name 插件名称 */
    void pluginUnloaded(const QString &name);
    /** @brief 插件加载错误 @param name 插件名称 @param error 错误信息 */
    void loadError(const QString &name, const QString &error);

private:
    QStringList m_searchPaths;                       ///< 插件搜索路径列表
    QMap<QString, QPluginLoader *> m_loaders;        ///< 插件名称到加载器的映射
    QMap<QString, PluginInfo> m_plugins;             ///< 插件名称到信息的映射

    PluginLoaderStats m_stats;                       ///< 加载器统计计数器
};
