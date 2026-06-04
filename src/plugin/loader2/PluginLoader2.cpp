/**
 * @file PluginLoader2.cpp
 * @brief 插件加载器v2实现 — 带元数据扫描的Qt插件动态加载/卸载
 */
#include "plugin/loader2/PluginLoader2.h"
#include <QDir>
#include <QJsonDocument>

/** @brief 构造函数 @param parent 父对象 */
PluginLoader::PluginLoader(QObject *parent) : QObject(parent) {}
/** @brief 析构函数，自动卸载所有已加载插件 */
PluginLoader::~PluginLoader() { unloadAll(); }

/** @brief 添加插件搜索路径 @param p 目录路径 */
void PluginLoader::addSearchPath(const QString &p) { if (!m_searchPaths.contains(p)) m_searchPaths.append(p); }
/** @brief 移除插件搜索路径 @param p 目录路径 */
void PluginLoader::removeSearchPath(const QString &p) { m_searchPaths.removeAll(p); }

/** @brief 加载指定路径的插件 @param fp 插件文件路径 @return 加载成功返回true */
bool PluginLoader::loadPlugin(const QString &fp) {
    ++m_stats.totalLoadAttempts;
    auto *loader = new QPluginLoader(fp, this);
    if (!loader->load()) {
        ++m_stats.totalLoadFailures;
        emit pluginError(fp, loader->errorString());
        delete loader;
        return false;
    }
    ++m_stats.totalLoadSuccesses;
    PluginEntry entry;
    entry.filePath = fp;
    auto meta = loader->metaData();
    entry.name = meta.value("MetaData").toObject().value("name").toString(QFileInfo(fp).baseName());
    entry.loader = loader;
    entry.instance = loader->instance();
    m_plugins[entry.name] = entry;
    emit pluginLoaded(entry.name, entry.filePath);
    return true;
}

/** @brief 卸载指定插件 @param name 插件名称 */
void PluginLoader::unloadPlugin(const QString &name) {
    auto it = m_plugins.find(name);
    if (it != m_plugins.end()) {
        ++m_stats.totalUnloads;
        if (it.value().loader) {
            it.value().loader->unload();
            delete it.value().loader;
        }
        m_plugins.erase(it);
        emit pluginUnloaded(name);
    }
}

/** @brief 获取插件实例对象 @param name 插件名称 @return QObject指针，不存在返回nullptr */
QObject *PluginLoader::pluginInstance(const QString &name) const {
    auto it = m_plugins.constFind(name);
    return it != m_plugins.constEnd() ? it.value().instance : nullptr;
}

/** @brief 获取所有已加载插件名称列表 @return 名称列表 */
QStringList PluginLoader::loadedPlugins() const { return m_plugins.keys(); }

/** @brief 查询插件是否已加载 @param name 插件名称 @return 是否已加载 */
bool PluginLoader::isLoaded(const QString &name) const { return m_plugins.contains(name); }

/** @brief 获取所有搜索路径 @return 路径列表 */
QStringList PluginLoader::searchPaths() const { return m_searchPaths; }

/** @brief 获取所有已扫描但未加载的插件名称列表 @return 名称列表 */
QStringList PluginLoader::discoveredPlugins() const { return m_discoveredPlugins.keys(); }

/** @brief 查询插件是否已被扫描发现(未加载) @param name 插件名称 @return 是否已发现 */
bool PluginLoader::isDiscovered(const QString &name) const { return m_discoveredPlugins.contains(name); }

/** @brief 获取已发现插件的文件路径 @param name 插件名称 @return 文件路径，未找到返回空 */
QString PluginLoader::discoveredPluginPath(const QString &name) const {
    auto it = m_discoveredPlugins.constFind(name);
    return it != m_discoveredPlugins.constEnd() ? it.value().filePath : QString();
}

/** @brief 扫描指定目录下的所有插件文件，记录元数据到m_discoveredPlugins但不实际加载 @param path 目录路径 */
void PluginLoader::scanDirectory(const QString &path) {
    ++m_stats.totalScanRuns;
    ++m_stats.totalDirectoriesScanned;
    QDir d(path);
    for (const auto &fi : d.entryInfoList(QStringList() << "*.dll" << "*.so" << "*.dylib", QDir::Files)) {
        ++m_stats.totalPluginsDiscovered;
        QPluginLoader loader(fi.absoluteFilePath());
        auto meta = loader.metaData();
        PluginEntry entry;
        entry.filePath = fi.absoluteFilePath();
        entry.name = meta.value("MetaData").toObject().value("name").toString(fi.baseName());
        entry.instance = nullptr;
        entry.loader = nullptr;
        /* 记录到已发现列表(不实际加载)，供后续loadPlugin使用 */
        m_discoveredPlugins[entry.name] = entry;
        emit pluginFound(entry.name, entry.filePath);
        loader.unload();
    }
}

/** @brief 卸载所有已加载插件并释放资源 */
void PluginLoader::unloadAll() {
    for (auto it = m_plugins.begin(); it != m_plugins.end(); ++it) {
        if (it.value().loader) {
            it.value().loader->unload();
            delete it.value().loader;
        }
        emit pluginUnloaded(it.key());
    }
    m_stats.totalUnloads += static_cast<quint64>(m_plugins.size());
    m_plugins.clear();
}
