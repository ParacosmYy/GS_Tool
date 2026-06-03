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

    /** @brief 获取已加载插件的实例对象 @param name 插件名称 @return 插件实例指针 */
    QObject* pluginInstance(const QString &name) const;
    /** @brief 获取所有已加载插件名称列表 @return 名称列表 */
    QStringList loadedPlugins() const;
    /** @brief 查询插件是否已加载 @param name 插件名称 @return 是否已加载 */
    bool isLoaded(const QString &name) const;

    /** @brief 扫描指定目录下的所有插件文件并尝试加载 @param path 目录路径 */
    void scanDirectory(const QString &path);

signals:
    /** @brief 插件加载完成时发射 @param name 插件名称 @param filePath 文件路径 */
    void pluginLoaded(const QString &name, const QString &filePath);
    /** @brief 插件卸载完成时发射 @param name 插件名称 */
    void pluginUnloaded(const QString &name);
    /** @brief 插件操作出错时发射 @param name 插件名称 @param error 错误信息 */
    void pluginError(const QString &name, const QString &error);

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

    QMap<QString, PluginEntry> m_plugins;  ///< 插件名称到条目的映射
    QStringList m_searchPaths;              ///< 插件搜索路径列表
};
