/**
 * @file PluginManager.cpp
 * @brief 插件管理器实现 — 骨架文件
 */

#include "plugin/PluginManager.h"
#include "plugin/PluginApi.h"

/**
 * @brief 构造函数
 *
 * 创建 PluginApi 实例供插件使用。
 *
 * @param parent 父对象
 */
PluginManager::PluginManager(QObject* parent)
    : QObject(parent)
    , m_api(nullptr)
{
}

/** @brief 析构函数，自动卸载所有插件 */
PluginManager::~PluginManager()
{
    unloadAll();
}

/**
 * @brief 扫描目录中的插件文件
 *
 * 搜索指定目录下符合插件命名规则的 DLL 文件。
 *
 * @param pluginDir 插件目录路径
 * @return 找到的插件文件路径列表
 */
QStringList PluginManager::scanPlugins(const QString& pluginDir)
{
    Q_UNUSED(pluginDir)
    // TODO: 扫描目录，过滤 DLL 文件
    return QStringList();
}

/**
 * @brief 加载指定插件
 *
 * 加载 DLL，调用 embedDebugPluginInit 入口函数获取插件接口，
 * 调用 initialize() 初始化插件。
 *
 * @param filePath 插件 DLL 文件路径
 * @return true 加载成功，false 加载失败
 */
bool PluginManager::loadPlugin(const QString& filePath)
{
    Q_UNUSED(filePath)
    // TODO: 使用 QLibrary 加载 DLL，解析入口函数
    return false;
}

/**
 * @brief 卸载指定插件
 *
 * 调用插件的 shutdown() 方法，然后卸载 DLL。
 *
 * @param name 插件名称
 */
void PluginManager::unloadPlugin(const QString& name)
{
    Q_UNUSED(name)
    // TODO: 调用 shutdown()，移除映射，卸载 DLL
}

/**
 * @brief 卸载所有已加载的插件
 */
void PluginManager::unloadAll()
{
    // TODO: 遍历 m_plugins，逐个卸载
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
