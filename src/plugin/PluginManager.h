/**
 * @file PluginManager.h
 * @brief 插件管理器 — 动态加载/卸载第三方插件并管理其生命周期
 *
 * 扫描指定目录的DLL文件，加载实现IEmbedDebugPlugin接口的插件。
 * 协作: IEmbedDebugPlugin(接口) / PluginApi(宿主API) / PluginConfigPanel(UI)
 */
#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include <QObject>
#include <QMap>
#include <QStringList>
#include <QVariantList>
#include "plugin/IEmbedDebugPlugin.h"

class PluginApi;

/** @brief 插件管理器 — 负责插件的发现、加载、初始化和卸载 */
class PluginManager : public QObject {
    Q_OBJECT

public:
    explicit PluginManager(QObject* parent = nullptr);
    ~PluginManager() override;

    QStringList scanPlugins(const QString& pluginDir); ///< 扫描目录中的插件文件
    bool loadPlugin(const QString& filePath);          ///< 加载指定插件
    void unloadPlugin(const QString& name);            ///< 卸载指定插件
    void unloadAll();                                  ///< 卸载所有已加载插件
    QStringList loadedPluginNames() const;             ///< 获取所有已加载插件名称
    IEmbedDebugPlugin* plugin(const QString& name) const; ///< 获取指定插件接口指针
    QString pluginVersion(const QString& name) const;  ///< 获取指定插件版本号
    QString pluginDescription(const QString& name) const; ///< 获取指定插件描述
    int pluginCount() const;                           ///< 获取已加载插件数量
    bool isPluginLoaded(const QString& name) const;    ///< 检查插件是否已加载
    QVariantList pluginMetadataList() const;           ///< 获取所有插件综合信息列表

    quint64 totalLoadCount() const;       ///< 获取累计加载成功次数
    quint64 totalFailCount() const;       ///< 获取累计加载失败次数
    quint64 totalUnloadCount() const;     ///< 获取累计卸载次数
    void resetLoadStatistics();
    quint64 totalScanCount() const { return m_scanCount; }
    quint64 totalDiscoveredFiles() const { return m_discoveredFiles; }
    quint64 totalPluginLoads() const { return m_loadCount; }
    quint64 totalPluginUnloads() const { return m_unloadCount; }
    quint64 totalScanAttempts() const { return m_scanCount; }
    quint64 totalLoadErrors() const { return m_failCount; }
    void resetStats();

signals:
    void pluginLoaded(const QString& name);
    void pluginUnloaded(const QString& name);
    void pluginError(const QString& name, const QString& error);

private:
    QMap<QString, IEmbedDebugPlugin*> m_plugins;  ///< 插件名称→接口映射
    PluginApi* m_api = nullptr;
    quint64 m_loadCount = 0, m_failCount = 0, m_unloadCount = 0;
    quint64 m_scanCount = 0, m_discoveredFiles = 0;
};

#endif // PLUGINMANAGER_H
