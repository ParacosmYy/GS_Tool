#include "core/factory/WidgetFactory.h"
#include <QWidget>

WidgetFactory::WidgetFactory(QObject *parent) : QObject(parent) {}
WidgetFactory::~WidgetFactory() = default;

void WidgetFactory::registerType(const QString &typeName, WidgetCreator creator)
{
    m_creators[typeName] = creator;
}

void WidgetFactory::unregisterType(const QString &typeName)
{
    m_creators.remove(typeName);
}

QWidget* WidgetFactory::create(const QString &typeName, QWidget *parent) const
{
    auto it = m_creators.constFind(typeName);
    if (it != m_creators.constEnd()) {
        return it.value()(parent);
    }
    return nullptr;
}

bool WidgetFactory::isRegistered(const QString &typeName) const
{
    return m_creators.contains(typeName);
}

QStringList WidgetFactory::registeredTypes() const
{
    return m_creators.keys();
}