#pragma once
#include <QObject>
#include <QMap>
#include <QString>
#include <QPluginLoader>
#include <QList>

class PluginLoader : public QObject {
    Q_OBJECT
public:
    struct PluginInfo {
        QString name;
        QString version;
        QString description;
        QString filePath;
        bool loaded = false;
    };
    explicit PluginLoader(QObject *parent = nullptr);
    ~PluginLoader() override;
    void addSearchPath(const QString &path);
    void removeSearchPath(const QString &path);
    QList<PluginInfo> scanPlugins();
    bool loadPlugin(const QString &filePath);
    void unloadPlugin(const QString &name);
    QObject *pluginInstance(const QString &name) const;
    QList<PluginInfo> loadedPlugins() const;
    QStringList searchPaths() const;
    void unloadAll();
signals:
    void pluginFound(const PluginInfo &info);
    void pluginLoaded(const QString &name);
    void pluginUnloaded(const QString &name);
    void loadError(const QString &name, const QString &error);
private:
    QStringList m_searchPaths;
    QMap<QString, QPluginLoader *> m_loaders;
    QMap<QString, PluginInfo> m_plugins;
};
