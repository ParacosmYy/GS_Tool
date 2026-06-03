/**
 * @file PluginManager.h
 * @brief 插件管理器 — 动态加载/卸载第三方插件并管理其生命周期
 *
 * 扫描指定目录的 DLL 文件，加载实现 IEmbedDebugPlugin 接口的插件，
 * 管理插件的初始化、运行和关闭过程。
 *
 * 协作关系:
 *   - IEmbedDebugPlugin: 插件接口契约
 *   - PluginApi: 提供给插件的宿主 API
 *   - PluginConfigPanel: 插件管理 UI
 */
#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include <QObject>
#include <QMap>
#include <QStringList>
#include <QVariantList>
#include "plugin/IEmbedDebugPlugin.h"

class PluginApi;

/**
 * @brief 插件管理器
 *
 * 负责插件的发现、加载、初始化和卸载。
 * 每个插件以名称为 key 存储在映射表中。
 */
class PluginManager : public QObject {
    Q_OBJECT

public:
    /** @brief 构造函数 */
    explicit PluginManager(QObject* parent = nullptr);

    /** @brief 析构函数，自动卸载所有插件 */
    ~PluginManager() override;

    /**
     * @brief 扫描目录中的插件文件
     * @param pluginDir 插件目录路径
     * @return 找到的插件文件路径列表
     */
    QStringList scanPlugins(const QString& pluginDir);

    /**
     * @brief 加载指定插件
     * @param filePath 插件 DLL 文件路径
     * @return true 加载成功，false 加载失败
     */
    bool loadPlugin(const QString& filePath);

    /**
     * @brief 卸载指定插件
     * @param name 插件名称
     */
    void unloadPlugin(const QString& name);

    /** @brief 卸载所有已加载的插件 */
    void unloadAll();

    /** @brief 获取所有已加载插件的名称列表 */
    QStringList loadedPluginNames() const;

    /**
     * @brief 获取指定插件的接口指针
     * @param name 插件名称
     * @return 插件接口指针，未找到返回 nullptr
     */
    IEmbedDebugPlugin* plugin(const QString& name) const;

    /**
     * @brief 获取指定插件的版本号
     * @param name 插件名称
     * @return 版本字符串，未找到返回空字符串
     */
    QString pluginVersion(const QString& name) const;

    /**
     * @brief 获取指定插件的描述
     * @param name 插件名称
     * @return 描述字符串，未找到返回空字符串
     */
    QString pluginDescription(const QString& name) const;

    /**
     * @brief 获取已加载插件数量
     * @return 插件数量
     */
    int pluginCount() const;

    /**
     * @brief 检查指定插件是否已加载
     * @param name 插件名称
     * @return true 已加载，false 未加载
     */
    bool isPluginLoaded(const QString& name) const;

    /**
     * @brief 获取所有插件的综合信息列表
     * @return QVariantList，每项包含 name/version/description
     */
    QVariantList pluginMetadataList() const;

signals:
    /** @brief 插件加载成功信号 */
    void pluginLoaded(const QString& name);

    /** @brief 插件卸载信号 */
    void pluginUnloaded(const QString& name);

    /**
     * @brief 插件错误信号
     * @param name 插件名称
     * @param error 错误描述
     */
    void pluginError(const QString& name, const QString& error);

private:
    QMap<QString, IEmbedDebugPlugin*> m_plugins;  ///< 插件名称→接口映射
    PluginApi* m_api = nullptr;                     ///< 宿主 API 实例
};

#endif // PLUGINMANAGER_H
