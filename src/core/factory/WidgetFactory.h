// Copyright 2024 EmbedDebug Project
// SPDX-License-Identifier: MIT
#pragma once
#include <QObject>
#include <QMap>
#include <QString>
#include <QStringList>
#include <functional>
#include <memory>

class QWidget;

using WidgetCreator = std::function<QWidget*(QWidget*)>;

class WidgetFactory : public QObject {
    Q_OBJECT
public:
    explicit WidgetFactory(QObject *parent = nullptr);
    ~WidgetFactory() override;

    void registerType(const QString &typeName, WidgetCreator creator);
    void unregisterType(const QString &typeName);
    QWidget* create(const QString &typeName, QWidget *parent = nullptr) const;
    bool isRegistered(const QString &typeName) const;
    QStringList registeredTypes() const;

    template<typename T>
    void registerType(const QString &typeName) {
        registerType(typeName, [](QWidget *p) -> QWidget* { return new T(p); });
    }

private:
    QMap<QString, WidgetCreator> m_creators;
};