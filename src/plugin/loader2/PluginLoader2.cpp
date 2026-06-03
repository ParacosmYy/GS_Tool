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

/** @brief 扫描所有搜索路径中的插件(读取元数据，不加载) @return 插件信息列表 */
QList<PluginLoader::PluginInfo> PluginLoader::scanPlugins() {
    QList<PluginInfo> result;
    for (const auto &dir : m_searchPaths) {
        QDir d(dir);
        for (const auto &fi : d.entryInfoList(QStringList() << "*.dll" << "*.so" << "*.dylib", QDir::Files)) {
            QPluginLoader loader(fi.absoluteFilePath());
            auto meta = loader.metaData();
            PluginInfo info;
            info.filePath = fi.absoluteFilePath();
            info.name = meta.value("MetaData").toObject().value("name").toString(fi.baseName());
            info.version = meta.value("MetaData").toObject().value("version").toString("0.0.0");
            info.description = meta.value("MetaData").toObject().value("description").toString();
            info.loaded = m_plugins.contains(info.name);
            result.append(info);
            emit pluginFound(info);
            loader.unload();
        }
    }
    return result;
}

/** @brief 加载指定路径的插件 @param fp 插件文件路径 @return 加载成功返回true */
bool PluginLoader::loadPlugin(const QString &fp) {
    auto *loader = new QPluginLoader(fp, this);
    if (!loader->load()) {
        emit loadError(fp, loader->errorString());
        delete loader;
        return false;
    }
    PluginInfo info;
    info.filePath = fp;
    auto meta = loader->metaData();
    info.name = meta.value("MetaData").toObject().value("name").toString(QFileInfo(fp).baseName());
    info.version = meta.value("MetaData").toObject().value("version").toString();
    info.loaded = true;
    m_loaders[info.name] = loader;
    m_plugins[info.name] = info;
    emit pluginLoaded(info.name);
    return true;
}

/** @brief 卸载指定插件 @param name 插件名称 */
void PluginLoader::unloadPlugin(const QString &name) {
    auto it = m_loaders.find(name);
    if (it != m_loaders.end()) { it.value()->unload(); delete it.value(); m_loaders.erase(it); m_plugins.remove(name); emit pluginUnloaded(name); }
}

/** @brief 获取插件实例对象 @param name 插件名称 @return QObject指针，不存在返回nullptr */
QObject *PluginLoader::pluginInstance(const QString &name) const {
    auto it = m_loaders.constFind(name);
    return it != m_loaders.constEnd() ? it.value()->instance() : nullptr;
}

/** @brief 获取所有已加载插件信息 @return 插件信息列表 */
QList<PluginLoader::PluginInfo> PluginLoader::loadedPlugins() const { return m_plugins.values(); }
/** @brief 获取所有搜索路径 @return 路径列表 */
QStringList PluginLoader::searchPaths() const { return m_searchPaths; }
/** @brief 卸载所有已加载插件并释放资源 */
void PluginLoader::unloadAll() { for (auto it = m_loaders.begin(); it != m_loaders.end(); ++it) { it.value()->unload(); emit pluginUnloaded(it.key()); } qDeleteAll(m_loaders); m_loaders.clear(); m_plugins.clear(); }
