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

// --- 便捷方法实现 ---

void SettingsManager::saveSerialConfig(const QVariantMap& config)
{
    // 使用 "serial/" 前缀分组存储串口配置
    m_settings.beginGroup("serial");
    // 先清除旧配置, 避免残留已删除的字段
    m_settings.remove(QString());
    // 逐项写入新配置
    for (auto it = config.constBegin(); it != config.constEnd(); ++it) {
        m_settings.setValue(it.key(), it.value());
    }
    m_settings.endGroup();
    m_settings.sync();
}

QVariantMap SettingsManager::loadSerialConfig() const
{
    QVariantMap result;
    // QSettings::beginGroup/endGroup 是非const方法, 这里需要const_cast
    // 仅做读取操作, 不会修改m_settings的实际内容
    QSettings& settings = const_cast<QSettings&>(m_settings);
    settings.beginGroup("serial");
    const QStringList keys = settings.childKeys();
    for (const QString& key : keys) {
        result.insert(key, settings.value(key));
    }
    settings.endGroup();
    return result;
}

void SettingsManager::saveWindowGeometry(const QByteArray& geometry)
{
    // 使用 "window/" 前缀存储窗口几何信息
    m_settings.setValue("window/geometry", geometry);
    m_settings.sync();
}

QByteArray SettingsManager::loadWindowGeometry() const
{
    return m_settings.value("window/geometry").toByteArray();
}

void SettingsManager::saveTheme(const QString& themeName)
{
    // 使用 "theme/" 前缀存储主题选择
    m_settings.setValue("theme/name", themeName);
    m_settings.sync();
}

QString SettingsManager::loadTheme() const
{
    // 不存在时返回默认主题
    return m_settings.value("theme/name", App::DEFAULT_THEME).toString();
}

void SettingsManager::saveLanguage(const QString& langCode)
{
    m_settings.setValue("language/code", langCode);
    m_settings.sync();
}

QString SettingsManager::loadLanguage() const
{
    return m_settings.value("language/code", Language::DEFAULT).toString();
}
