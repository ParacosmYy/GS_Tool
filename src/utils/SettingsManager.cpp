#include "SettingsManager.h"
#include "core/Constants.h"

SettingsManager& SettingsManager::instance()
{
    static SettingsManager inst;
    return inst;
}

SettingsManager::SettingsManager(QObject* parent)
    : QObject(parent)
    , m_settings(App::SETTINGS_FILE, QSettings::IniFormat)
{
    m_settings.setFallbacksEnabled(true);
}

SettingsManager::~SettingsManager()
{
    m_settings.sync();
}

QVariant SettingsManager::get(const QString& key, const QVariant& defaultValue) const
{
    return m_settings.value(key, defaultValue);
}

void SettingsManager::set(const QString& key, const QVariant& value)
{
    m_settings.setValue(key, value);
}

void SettingsManager::remove(const QString& key)
{
    m_settings.remove(key);
}

bool SettingsManager::contains(const QString& key) const
{
    return m_settings.contains(key);
}

void SettingsManager::sync()
{
    m_settings.sync();
}
