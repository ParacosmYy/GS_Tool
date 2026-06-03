// Copyright 2024 EmbedDebug Project
// SPDX-License-Identifier: MIT
#pragma once
#include <QObject>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QPluginLoader>

class PluginLoader : public QObject {
    Q_OBJECT
public:
    explicit PluginLoader(QObject *parent = nullptr);
    ~PluginLoader() override;

    void addSearchPath(const QString &path);
    void removeSearchPath(const QString &path);
    QStringList searchPaths() const;

    bool loadPlugin(const QString &filePath);
    void unloadPlugin(const QString &name);
    void unloadAll();

    QObject* pluginInstance(const QString &name) const;
    QStringList loadedPlugins() const;
    bool isLoaded(const QString &name) const;

    void scanDirectory(const QString &path);

signals:
    void pluginLoaded(const QString &name, const QString &filePath);
    void pluginUnloaded(const QString &name);
    void pluginError(const QString &name, const QString &error);

private:
    struct PluginEntry {
        QString name;
        QString filePath;
        QPluginLoader *loader = nullptr;
        QObject *instance = nullptr;
    };
    QMap<QString, PluginEntry> m_plugins;
    QStringList m_searchPaths;
};