/**
 * @file SettingsManager.cpp
 * @brief 配置管理器核心实现（单例、构造/析构、基本读写）
 *
 * 包含:
 *   - 单例模式: instance()
 *   - 构造/析构: SettingsManager() / ~SettingsManager()
 *   - 基本读写: get/set/remove/contains/sync
 *
 * 分组操作见 SettingsManagerGroups.cpp
 * 便捷方法和统计见 SettingsManagerConvenience.cpp
 */

#include "utils/settings/SettingsManager.h"
#include "shared/AppConstants.h"

// ============================================================
// 单例模式
// ============================================================

/**
 * @brief 获取全局唯一实例
 *
 * 使用 Meyers' Singleton 模式（C++11 保证线程安全的局部静态变量）。
 * 首次调用时构造，程序退出时自动析构。
 *
 * @return SettingsManager 的全局引用
 */
SettingsManager& SettingsManager::instance()
{
    static SettingsManager inst;
    return inst;
}

// ============================================================
// 构造 / 析构
// ============================================================

/**
 * @brief 私有构造函数
 *
 * 使用 IniFormat 格式存储配置文件，文件名由 App::SETTINGS_FILE 定义。
 * 启用 fallbacks 机制以支持平台特定的配置路径回退。
 *
 * @param parent  父对象，用于 QObject 父子树管理
 */
SettingsManager::SettingsManager(QObject* parent)
    : QObject(parent)
    , m_settings(App::SETTINGS_FILE, QSettings::IniFormat)
{
    m_settings.setFallbacksEnabled(true);
}

/**
 * @brief 析构函数
 *
 * 自动退出所有未关闭的分组（防止遗漏 endGroup 导致状态不一致），
 * 然后将配置同步写入磁盘。
 */
SettingsManager::~SettingsManager()
{
    // 安全退出所有未关闭的分组
    while (!m_groupStack.isEmpty()) {
        m_settings.endGroup();
        m_groupStack.pop();
    }
    m_settings.sync();
}

// ============================================================
// 基本读写接口
// ============================================================

/**
 * @brief 读取配置值
 * @param key          配置键名
 * @param defaultValue 默认值
 * @return             配置值或默认值
 */
QVariant SettingsManager::get(const QString& key, const QVariant& defaultValue) const
{
    ++m_totalReads;  ///< 统计: 配置读取次数递增
    return m_settings.value(key, defaultValue);
}

/**
 * @brief 写入配置值
 * @param key    配置键名
 * @param value  要写入的值
 */
void SettingsManager::set(const QString& key, const QVariant& value)
{
    ++m_totalWrites;  ///< 统计: 配置写入次数递增
    m_settings.setValue(key, value);
}

/**
 * @brief 删除单个配置项
 * @param key  要删除的配置键名
 */
void SettingsManager::remove(const QString& key)
{
    ++m_totalRemoves;  ///< 统计: 配置删除次数递增
    m_settings.remove(key);
}

/**
 * @brief 检查配置项是否存在
 *
 * 在当前分组上下文中查找。如果当前在 "serial" 分组中，
 * contains("baudRate") 会查找 "serial/baudRate"。
 *
 * @param key  配置键名
 * @return     true 键存在，false 不存在
 */
bool SettingsManager::contains(const QString& key) const
{
    ++m_totalContainsChecks;
    return m_settings.contains(key);
}

/**
 * @brief 同步配置到磁盘
 *
 * 将内存中的配置变更写入磁盘文件。
 * 建议在关键配置变更后调用，避免异常退出时数据丢失。
 */
void SettingsManager::sync()
{
    ++m_totalSyncs;
    m_settings.sync();
}

// 分组操作和 RAII 守卫见 SettingsManagerGroups.cpp
// 便捷方法和统计接口见 SettingsManagerConvenience.cpp

