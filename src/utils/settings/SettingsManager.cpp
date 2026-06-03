/**
 * @file SettingsManager.cpp
 * @brief 配置管理器实现
 *
 * 实现 SettingsManager 的所有方法，包括:
 *   - 基本读写: get/set/remove/contains
 *   - 分组操作: beginGroup/endGroup/removeGroup/containsGroup
 *   - RAII 守卫: groupGuard
 *   - 便捷方法: 串口配置、窗口几何、主题、语言
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
    m_settings.sync();
}

// ============================================================
// 分组操作接口
// ============================================================

/**
 * @brief 进入配置分组
 *
 * 将分组名压入栈中，支持嵌套调用。
 * 进入分组后，所有 get/set/remove/contains 操作都在该分组下进行。
 *
 * 嵌套示例:
 *   beginGroup("app")    → 栈: ["app"]
 *   beginGroup("serial") → 栈: ["app", "serial"]
 *   set("baudRate", ...) → 写入 app/serial/baudRate
 *   endGroup()           → 栈: ["app"]
 *   endGroup()           → 栈: []
 *
 * @param group  分组名称（不含 "/" 分隔符）
 */
void SettingsManager::beginGroup(const QString& group)
{
    m_settings.beginGroup(group);
    m_groupStack.push(group);
}

/**
 * @brief 退出当前分组
 *
 * 从栈中弹出最后一个分组名，并调用 QSettings::endGroup。
 * 如果栈为空（没有活跃的分组），不做任何操作。
 */
void SettingsManager::endGroup()
{
    if (m_groupStack.isEmpty()) {
        return;
    }
    m_groupStack.pop();
    m_settings.endGroup();
}

/**
 * @brief 获取当前分组路径
 *
 * 将栈中所有分组名用 "/" 拼接，返回完整的分组路径。
 * 不在任何分组中时返回空字符串。
 *
 * @return 分组路径，如 "app/serial"
 */
QString SettingsManager::currentGroup() const
{
    return m_groupStack.isEmpty() ? QString() : m_groupStack.join('/');
}

/**
 * @brief 删除整个配置分组及其所有子项
 *
 * 使用 RAII 守卫进入分组，删除分组内的所有键值对，
 * 然后自动退出分组。不影响调用者当前的分组上下文。
 *
 * 注意: 此方法会删除分组下的直接子键，但不会递归删除子分组。
 * 如果需要递归删除，请先获取子分组列表并逐个删除。
 *
 * @param group  要删除的分组名称
 */
void SettingsManager::removeGroup(const QString& group)
{
    // 使用 RAII 守卫确保分组操作配对
    GroupGuard guard(*this, group);
    // 删除分组内的所有键值对（空字符串参数表示"当前分组下的所有键"）
    m_settings.remove(QString());
}

/**
 * @brief 检查指定分组是否存在
 *
 * 使用临时分组上下文查询，不影响当前的分组状态。
 * 检查该分组下是否有任何子键或子分组。
 *
 * @param group  分组名称
 * @return     true 分组存在且有内容，false 不存在或为空
 */
bool SettingsManager::containsGroup(const QString& group) const
{
    // 使用独立的QSettings实例查询，避免干扰m_settings的分组状态
    QSettings temp(m_settings.fileName(), m_settings.format());
    temp.beginGroup(group);
    bool hasKeys = !temp.childKeys().isEmpty();
    bool hasChildGroups = !temp.childGroups().isEmpty();
    return hasKeys || hasChildGroups;
}

/**
 * @brief 获取指定分组下的所有键名
 *
 * 使用临时分组上下文查询，不影响当前的分组状态。
 * 如果 group 为空字符串，返回根级别的所有键名。
 *
 * @param group  分组名称（空字符串表示根级别）
 * @return       键名列表
 */
QStringList SettingsManager::groupKeys(const QString& group) const
{
    // 使用独立的QSettings实例查询，避免干扰m_settings的分组状态
    QSettings temp(m_settings.fileName(), m_settings.format());
    if (!group.isEmpty()) {
        temp.beginGroup(group);
    }
    QStringList keys = temp.childKeys();
    return keys;
}

/**
 * @brief 获取指定分组下的所有子分组名
 *
 * 使用临时分组上下文查询，不影响当前的分组状态。
 * 如果 group 为空字符串，返回根级别的所有子分组名。
 *
 * @param group  分组名称（空字符串表示根级别）
 * @return       子分组名列表
 */
QStringList SettingsManager::childGroups(const QString& group) const
{
    // 使用独立的QSettings实例查询，避免干扰m_settings的分组状态
    QSettings temp(m_settings.fileName(), m_settings.format());
    if (!group.isEmpty()) {
        temp.beginGroup(group);
    }
    QStringList groups = temp.childGroups();
    return groups;
}

// ============================================================
// RAII 分组守卫
// ============================================================

/**
 * @brief 创建 RAII 分组守卫
 *
 * 推荐使用此方法进行分组操作。守卫对象在析构时自动调用 endGroup，
 * 避免因异常或提前返回导致分组未关闭。
 *
 * 使用示例:
 * @code
 *   {
 *       auto guard = SettingsManager::instance().groupGuard("serial");
 *       SettingsManager::instance().set("baudRate", 115200);
 *       SettingsManager::instance().set("portName", "COM3");
 *       // guard 析构时自动 endGroup
 *   }
 * @endcode
 *
 * @param group  分组名称
 * @return       GroupGuard 对象
 */
SettingsManager::GroupGuard SettingsManager::groupGuard(const QString& group)
{
    return GroupGuard(*this, group);
}

// 便捷方法和统计接口见 SettingsManagerConvenience.cpp

