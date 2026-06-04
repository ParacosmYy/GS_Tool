/**
 * @file PluginLoader.cpp
 * @brief 插件加载器实现 — Qt插件动态加载/卸载、目录扫描
 */
#include "plugin/loader/PluginLoader.h"
#include <QDir>
#include <QFileInfo>
#include <QJsonObject>
#include <QJsonArray>

using PluginInfo = PluginLoader::PluginInfo;

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
        ++m_totalLoadFailures;
        emit loadError(fi.fileName(), tr("File not found: %1").arg(filePath));
        return false;
    }
    auto *loader = new QPluginLoader(filePath, this);
    QJsonObject meta = loader->metaData();
    auto *instance = loader->instance();
    if (!instance) {
        QString err = loader->errorString();
        delete loader;
        ++m_totalLoadFailures;
        emit loadError(fi.fileName(), err);
        return false;
    }
    PluginInfo info;
    info.name = meta.value("Keys").toArray().first().toString(fi.baseName());
    info.version = meta.value("Version").toString();
    info.description = meta.value("Description").toString();
    info.filePath = filePath;
    info.loaded = true;
    m_loaders[info.name] = loader;
    m_plugins[info.name] = info;
    ++m_totalLoadSuccesses;
    emit pluginLoaded(info.name);
    return true;
}

/** @brief 卸载指定插件 @param name 插件名称 */
void PluginLoader::unloadPlugin(const QString &name)
{
    auto it = m_loaders.find(name);
    if (it != m_loaders.end()) {
        it.value()->unload();
        delete it.value();
        m_loaders.erase(it);
        m_plugins.remove(name);
        ++m_totalUnloads;
        emit pluginUnloaded(name);
    }
}

/** @brief 卸载所有已加载插件 */
void PluginLoader::unloadAll()
{
    for (auto it = m_loaders.begin(); it != m_loaders.end(); ++it) {
        it.value()->unload();
        delete it.value();
        emit pluginUnloaded(it.key());
    }
    m_loaders.clear();
    m_plugins.clear();
}

/** @brief 获取插件实例对象 @param name 插件名称 @return QObject指针，不存在返回nullptr */
QObject* PluginLoader::pluginInstance(const QString &name) const
{
    auto it = m_loaders.constFind(name);
    return (it != m_loaders.constEnd()) ? it.value()->instance() : nullptr;
}

/** @brief 获取所有已加载插件的信息 @return 插件信息列表 */
QList<PluginLoader::PluginInfo> PluginLoader::loadedPlugins() const
{
    QList<PluginInfo> result;
    for (auto it = m_plugins.constBegin(); it != m_plugins.constEnd(); ++it) {
        if (it.value().loaded) result.append(it.value());
    }
    return result;
}

/** @brief 扫描所有搜索路径中的插件 @return 发现的插件信息列表 */
QList<PluginLoader::PluginInfo> PluginLoader::scanPlugins()
{
    QList<PluginInfo> found;
    for (const QString& searchPath : m_searchPaths) {
        QDir dir(searchPath);
        const auto entries = dir.entryInfoList(
            QStringList() << "*.dll" << "*.so" << "*.dylib", QDir::Files);
        for (const auto& fi : entries) {
            PluginInfo info;
            info.name = fi.baseName();
            info.filePath = fi.absoluteFilePath();
            info.loaded = m_plugins.contains(info.name);
            found.append(info);
            emit pluginFound(info);
        }
    }
    return found;
}
