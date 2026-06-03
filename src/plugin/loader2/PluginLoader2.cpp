#include "plugin/loader2/PluginLoader2.h"
#include <QDir>
#include <QJsonDocument>

PluginLoader::PluginLoader(QObject *parent) : QObject(parent) {}
PluginLoader::~PluginLoader() { unloadAll(); }

void PluginLoader::addSearchPath(const QString &p) { if (!m_searchPaths.contains(p)) m_searchPaths.append(p); }
void PluginLoader::removeSearchPath(const QString &p) { m_searchPaths.removeAll(p); }

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

void PluginLoader::unloadPlugin(const QString &name) {
    auto it = m_loaders.find(name);
    if (it != m_loaders.end()) { it.value()->unload(); delete it.value(); m_loaders.erase(it); m_plugins.remove(name); emit pluginUnloaded(name); }
}

QObject *PluginLoader::pluginInstance(const QString &name) const {
    auto it = m_loaders.constFind(name);
    return it != m_loaders.constEnd() ? it.value()->instance() : nullptr;
}

QList<PluginLoader::PluginInfo> PluginLoader::loadedPlugins() const { return m_plugins.values(); }
QStringList PluginLoader::searchPaths() const { return m_searchPaths; }
void PluginLoader::unloadAll() { for (auto it = m_loaders.begin(); it != m_loaders.end(); ++it) { it.value()->unload(); emit pluginUnloaded(it.key()); } qDeleteAll(m_loaders); m_loaders.clear(); m_plugins.clear(); }
