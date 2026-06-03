/**
 * @file SettingsManagerConvenience.cpp
 * @brief SettingsManager 便捷方法和统计接口
 *
 * 从 SettingsManager.cpp 拆分而来，包含串口配置、窗口几何、
 * 主题、语言等便捷存取方法，以及统计计数器接口。
 */

#include "utils/settings/SettingsManager.h"
#include "shared/Constants.h"

// ============================================================
// 便捷方法: 串口配置
// ============================================================

/** @brief 保存串口配置到 "serial" 分组(先清除旧配置再逐项写入)
 * @param config 串口配置键值对(portName/baudRate/dataBits/parity/stopBits/flowControl) */
void SettingsManager::saveSerialConfig(const QVariantMap& config)
{
    GroupGuard guard(*this, "serial");
    m_settings.remove(QString());
    for (auto it = config.constBegin(); it != config.constEnd(); ++it) {
        m_settings.setValue(it.key(), it.value());
    }
    m_settings.sync();
}

/** @brief 加载串口配置(使用临时QSettings实例，不破坏全局分组栈) @return 配置键值对 */
QVariantMap SettingsManager::loadSerialConfig() const
{
    QVariantMap result;
    QSettings tempSettings(m_settings.fileName(), m_settings.format());
    tempSettings.beginGroup("serial");
    const QStringList keys = tempSettings.childKeys();
    for (const QString& key : keys) {
        result.insert(key, tempSettings.value(key));
    }
    tempSettings.endGroup();
    return result;
}

// ============================================================
// 便捷方法: 窗口几何
// ============================================================

/** @brief 保存窗口位置和大小 @param geometry 由QWidget::saveGeometry()生成 */
void SettingsManager::saveWindowGeometry(const QByteArray& geometry)
{
    m_settings.setValue("window/geometry", geometry);
    m_settings.sync();
}

/** @brief 加载窗口位置和大小 @return 几何信息，不存在返回空QByteArray */
QByteArray SettingsManager::loadWindowGeometry() const
{
    return m_settings.value("window/geometry").toByteArray();
}

// ============================================================
// 便捷方法: 主题
// ============================================================

/** @brief 保存主题名称 @param themeName 主题名称(如 "dark_terminal", "modern_dark") */
void SettingsManager::saveTheme(const QString& themeName)
{
    m_settings.setValue("theme/name", themeName);
    m_settings.sync();
}

/** @brief 加载主题名称 @return 主题名称，默认 "dark_terminal" */
QString SettingsManager::loadTheme() const
{
    return m_settings.value("theme/name", App::DEFAULT_THEME).toString();
}

// ============================================================
// 便捷方法: 语言
// ============================================================

/** @brief 保存语言代码 @param langCode 语言代码(如 "zh_CN" 或 "en") */
void SettingsManager::saveLanguage(const QString& langCode)
{
    m_settings.setValue("language/code", langCode);
    m_settings.sync();
}

/** @brief 加载语言代码 @return 语言代码，默认 "zh_CN" */
QString SettingsManager::loadLanguage() const
{
    return m_settings.value("language/code", Language::DEFAULT).toString();
}

// ============================================================
// 统计计数器
// ============================================================

/** @brief 获取配置读取总次数 @return 读取操作总次数 */
quint64 SettingsManager::totalReads() const { return m_totalReads; }

/** @brief 获取配置写入总次数 @return 写入操作总次数 */
quint64 SettingsManager::totalWrites() const { return m_totalWrites; }

/** @brief 获取配置删除总次数 @return 删除操作总次数 */
quint64 SettingsManager::totalRemoves() const { return m_totalRemoves; }

/** @brief 获取配置操作错误总次数 @return 错误次数 */
quint64 SettingsManager::errorCount() const { return m_errorCount; }

/** @brief 重置所有配置管理统计计数器为零 */
void SettingsManager::resetSettingsStatistics()
{
    m_totalReads = 0;
    m_totalWrites = 0;
    m_totalRemoves = 0;
    m_errorCount = 0;
}
