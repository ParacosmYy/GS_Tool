/**
 * @file PluginLoader.cpp
 * @brief 插件加载器实现 — Qt插件动态加载/卸载、目录扫描
 */
#include "plugin/loader/PluginLoader.h"
#include <QDir>
#include <QFileInfo>

/** @brief 构造函数 @param parent 父对象 */
PluginLoader::PluginLoader(QObject *parent) : QObject(parent) {}
/** @brief 析构函数，自动卸载所有已加载插件 */
PluginLoader::~PluginLoader() { unloadAll(); }

/** @brief 添加插件搜索路径 @param path 目录路径 */
void PluginLoader::addSearchPath(const QString &path)
{
    if (!m_searchPaths.contains(path)) m_searchPaths.append(path);
}

/** @brief 移除插件搜索路径 @param path 目录路径 */
void PluginLoader::removeSearchPath(const QString &path) { m_searchPaths.removeAll(path); }
/** @brief 获取所有搜索路径 @return 路径列表 */
QStringList PluginLoader::searchPaths() const { return m_searchPaths; }

/** @brief 加载指定路径的插件 @param filePath 插件文件路径 @return 加载成功返回true */
bool PluginLoader::loadPlugin(const QString &filePath)
{
    QFileInfo fi(filePath);
    if (!fi.exists()) {
        emit pluginError(fi.fileName(), tr("File not found: %1").arg(filePath));
        return false;
    }
    auto *loader = new QPluginLoader(filePath, this);
    auto *instance = loader->instance();
    if (!instance) {
        QString err = loader->errorString();
        delete loader;
        emit pluginError(fi.fileName(), err);
        return false;
    }
    PluginEntry entry;
    entry.name = fi.baseName();
    entry.filePath = filePath;
    entry.loader = loader;
    entry.instance = instance;
    m_plugins[entry.name] = entry;
    emit pluginLoaded(entry.name, filePath);
    return true;
}

/** @brief 卸载指定插件 @param name 插件名称 */
void PluginLoader::unloadPlugin(const QString &name)
{
    auto it = m_plugins.find(name);
    if (it != m_plugins.end()) {
        it->loader->unload();
        delete it->loader;
        m_plugins.erase(it);
        emit pluginUnloaded(name);
    }
}

/** @brief 卸载所有已加载插件 */
void PluginLoader::unloadAll()
{
    for (auto it = m_plugins.begin(); it != m_plugins.end(); ++it) {
        it->loader->unload();
        delete it->loader;
        emit pluginUnloaded(it.key());
    }
    m_plugins.clear();
}

/** @brief 获取插件实例对象 @param name 插件名称 @return QObject指针，不存在返回nullptr */
QObject* PluginLoader::pluginInstance(const QString &name) const
{
    auto it = m_plugins.constFind(name);
    return (it != m_plugins.constEnd()) ? it->instance : nullptr;
}

/** @brief 获取所有已加载插件名称 @return 名称列表 */
QStringList PluginLoader::loadedPlugins() const { return m_plugins.keys(); }
/** @brief 检查插件是否已加载 @param name 插件名称 @return 已加载返回true */
bool PluginLoader::isLoaded(const QString &name) const { return m_plugins.contains(name); }

/** @brief 扫描目录下的所有插件文件并尝试加载 @param path 目录路径 */
void PluginLoader::scanDirectory(const QString &path)
{
    QDir dir(path);
    const auto entries = dir.entryInfoList(QStringList() << "*.dll" << "*.so" << "*.dylib",
        QDir::Files);
    for (const auto &fi : entries) {
        loadPlugin(fi.absoluteFilePath());
    }
}
