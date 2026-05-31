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

#include "SettingsManager.h"
#include "core/Constants.h"

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
    return m_settings.value(key, defaultValue);
}

/**
 * @brief 写入配置值
 * @param key    配置键名
 * @param value  要写入的值
 */
void SettingsManager::set(const QString& key, const QVariant& value)
{
    m_settings.setValue(key, value);
}

/**
 * @brief 删除单个配置项
 * @param key  要删除的配置键名
 */
void SettingsManager::remove(const QString& key)
{
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
    // QSettings 的 beginGroup/endGroup 是非 const 方法，需要 const_cast
    // 仅做读取操作，不会修改 m_settings 的实际内容
    QSettings& settings = const_cast<QSettings&>(m_settings);

    // 保存当前分组状态
    QString originalGroup = settings.group();

    settings.beginGroup(group);
    bool hasKeys = !settings.childKeys().isEmpty();
    bool hasChildGroups = !settings.childGroups().isEmpty();
    settings.endGroup();

    // 恢复原始分组状态
    // 如果原来就在某个分组中，需要重新进入
    if (!originalGroup.isEmpty()) {
        settings.beginGroup(originalGroup);
    }

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
    QSettings& settings = const_cast<QSettings&>(m_settings);

    // 保存当前分组状态
    QString originalGroup = settings.group();

    // 如果指定了分组，进入该分组
    if (!group.isEmpty()) {
        settings.beginGroup(group);
    }

    QStringList keys = settings.childKeys();

    // 恢复原始分组状态
    if (!group.isEmpty()) {
        settings.endGroup();
    }
    if (!originalGroup.isEmpty()) {
        settings.beginGroup(originalGroup);
    }

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
    QSettings& settings = const_cast<QSettings&>(m_settings);

    // 保存当前分组状态
    QString originalGroup = settings.group();

    // 如果指定了分组，进入该分组
    if (!group.isEmpty()) {
        settings.beginGroup(group);
    }

    QStringList groups = settings.childGroups();

    // 恢复原始分组状态
    if (!group.isEmpty()) {
        settings.endGroup();
    }
    if (!originalGroup.isEmpty()) {
        settings.beginGroup(originalGroup);
    }

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

// ============================================================
// 便捷方法: 串口配置
// ============================================================

/**
 * @brief 保存串口配置
 *
 * 将完整的串口配置写入 "serial" 分组。
 * 先清除旧配置再写入新配置，避免残留已删除的字段。
 *
 * @param config  串口配置键值对，典型键名:
 *                - "portName": 端口名（如 "COM3"）
 *                - "baudRate": 波特率（如 115200）
 *                - "dataBits": 数据位（5/6/7/8）
 *                - "parity":   校验位（0=None, 2=Even, 3=Odd）
 *                - "stopBits": 停止位（1/2）
 *                - "flowControl": 流控（0=None, 1=Hardware, 2=Software）
 */
void SettingsManager::saveSerialConfig(const QVariantMap& config)
{
    // 使用 RAII 守卫确保分组操作配对
    GroupGuard guard(*this, "serial");
    // 先清除旧配置，避免残留已删除的字段
    m_settings.remove(QString());
    // 逐项写入新配置
    for (auto it = config.constBegin(); it != config.constEnd(); ++it) {
        m_settings.setValue(it.key(), it.value());
    }
    m_settings.sync();
}

/**
 * @brief 加载串口配置
 * @return 串口配置键值对，不存在时返回空 map
 */
QVariantMap SettingsManager::loadSerialConfig() const
{
    QVariantMap result;
    // QSettings::beginGroup/endGroup 是非 const 方法，需要 const_cast
    // 仅做读取操作，不会修改 m_settings 的实际内容
    QSettings& settings = const_cast<QSettings&>(m_settings);
    settings.beginGroup("serial");
    const QStringList keys = settings.childKeys();
    for (const QString& key : keys) {
        result.insert(key, settings.value(key));
    }
    settings.endGroup();
    return result;
}

// ============================================================
// 便捷方法: 窗口几何
// ============================================================

/**
 * @brief 保存窗口位置和大小
 * @param geometry  由 QWidget::saveGeometry() 生成的字节数组
 */
void SettingsManager::saveWindowGeometry(const QByteArray& geometry)
{
    m_settings.setValue("window/geometry", geometry);
    m_settings.sync();
}

/**
 * @brief 加载窗口位置和大小
 * @return 窗口几何信息，不存在时返回空 QByteArray
 */
QByteArray SettingsManager::loadWindowGeometry() const
{
    return m_settings.value("window/geometry").toByteArray();
}

// ============================================================
// 便捷方法: 主题
// ============================================================

/**
 * @brief 保存主题名称
 * @param themeName  主题名称（如 "dark_terminal", "modern_dark", "light"）
 */
void SettingsManager::saveTheme(const QString& themeName)
{
    m_settings.setValue("theme/name", themeName);
    m_settings.sync();
}

/**
 * @brief 加载主题名称
 * @return 主题名称，不存在时返回 App::DEFAULT_THEME（"dark_terminal"）
 */
QString SettingsManager::loadTheme() const
{
    return m_settings.value("theme/name", App::DEFAULT_THEME).toString();
}

// ============================================================
// 便捷方法: 语言
// ============================================================

/**
 * @brief 保存语言选择
 * @param langCode  语言代码（"zh_CN" 或 "en"）
 */
void SettingsManager::saveLanguage(const QString& langCode)
{
    m_settings.setValue("language/code", langCode);
    m_settings.sync();
}

/**
 * @brief 加载语言选择
 * @return 语言代码，不存在时返回默认语言 "zh_CN"
 */
QString SettingsManager::loadLanguage() const
{
    return m_settings.value("language/code", Language::DEFAULT).toString();
}
