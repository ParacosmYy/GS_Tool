#include "plugin/loader/PluginLoader.h"
#include <QDir>
#include <QFileInfo>

PluginLoader::PluginLoader(QObject *parent) : QObject(parent) {}
PluginLoader::~PluginLoader() { unloadAll(); }

void PluginLoader::addSearchPath(const QString &path)
{
    if (!m_searchPaths.contains(path)) m_searchPaths.append(path);
}

void PluginLoader::removeSearchPath(const QString &path) { m_searchPaths.removeAll(path); }
QStringList PluginLoader::searchPaths() const { return m_searchPaths; }

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

void PluginLoader::unloadAll()
{
    for (auto it = m_plugins.begin(); it != m_plugins.end(); ++it) {
        it->loader->unload();
        delete it->loader;
        emit pluginUnloaded(it.key());
    }
    m_plugins.clear();
}

QObject* PluginLoader::pluginInstance(const QString &name) const
{
    auto it = m_plugins.constFind(name);
    return (it != m_plugins.constEnd()) ? it->instance : nullptr;
}

QStringList PluginLoader::loadedPlugins() const { return m_plugins.keys(); }
bool PluginLoader::isLoaded(const QString &name) const { return m_plugins.contains(name); }

void PluginLoader::scanDirectory(const QString &path)
{
    QDir dir(path);
    const auto entries = dir.entryInfoList(QStringList() << "*.dll" << "*.so" << "*.dylib",
        QDir::Files);
    for (const auto &fi : entries) {
        loadPlugin(fi.absoluteFilePath());
    }
}