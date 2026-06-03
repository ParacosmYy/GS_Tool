/**
 * @file SettingsManager.h
 * @brief 配置管理器 - 单例，封装 QSettings 提供类型安全的配置读写
 *
 * 设计要点:
 *   - 单例模式，全局唯一实例，通过 instance() 获取
 *   - 使用 IniFormat 存储配置文件
 *   - 支持嵌套分组: beginGroup/endGroup 可嵌套，RAII 守卫确保不遗漏
 *   - 提供串口配置、窗口几何、主题、语言等常用配置的便捷方法
 *
 * 线程安全: QSettings 本身线程安全，但 beginGroup/endGroup 不支持多线程并发。
 */
#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QObject>
#include <QSettings>
#include <QVariant>
#include <QVariantMap>
#include <QByteArray>
#include <QString>
#include <QStringList>
#include <QStack>

/**
 * @brief 配置管理器 - 单例，封装 QSettings 提供类型安全的配置读写
 *
 * 使用示例:
 * @code
 *   SettingsManager::instance().set("key", value);
 *   QVariant val = SettingsManager::instance().get("key", defaultValue);
 *
 *   // RAII 守卫（推荐）
 *   auto guard = SettingsManager::instance().groupGuard("serial");
 *   SettingsManager::instance().set("baudRate", 115200);
 *   // guard 析构时自动 endGroup
 * @endcode
 */
class SettingsManager : public QObject {
    Q_OBJECT

public:
    /// 获取全局唯一实例
    static SettingsManager& instance();

    // ---- 基本读写接口 ----

    /// 读取配置值，支持 "/" 分隔符访问嵌套键（如 "serial/baudRate"）
    QVariant get(const QString& key, const QVariant& defaultValue = QVariant()) const;

    /// 写入配置值
    void set(const QString& key, const QVariant& value);

    /// 删除单个配置项
    void remove(const QString& key);

    /// 检查配置项是否存在（在当前分组上下文中查找）
    bool contains(const QString& key) const;

    /// 将配置同步写入磁盘（析构函数中也会自动调用）
    void sync();

    // ---- 分组操作接口 ----

    /// 进入配置分组，支持嵌套调用（beginGroup("app"); beginGroup("serial"); → "app/serial"）
    void beginGroup(const QString& group);

    /// 退出当前分组（必须与 beginGroup 配对调用，无活跃分组时无效）
    void endGroup();

    /// 获取当前分组路径（如 "app/serial"），不在任何分组中时返回空字符串
    QString currentGroup() const;

    /// 删除整个配置分组及其所有子项（不影响当前分组上下文）
    void removeGroup(const QString& group);

    /// 检查指定分组是否存在且有内容（使用临时分组上下文查询）
    bool containsGroup(const QString& group) const;

    /// 获取指定分组下的所有键名（空字符串表示根级别）
    QStringList groupKeys(const QString& group = QString()) const;

    /// 获取指定分组下的所有子分组名（空字符串表示根级别）
    QStringList childGroups(const QString& group = QString()) const;

    // ---- RAII 分组守卫 ----

    /**
     * @brief RAII 分组守卫 - 构造时 beginGroup，析构时 endGroup
     *
     * 确保异常或提前返回时不会遗漏 endGroup。禁止拷贝和移动。
     */
    class GroupGuard {
    public:
        GroupGuard(SettingsManager& settings, const QString& group)
            : m_settings(settings) { m_settings.beginGroup(group); }
        ~GroupGuard() { m_settings.endGroup(); }

        GroupGuard(const GroupGuard&) = delete;
        GroupGuard& operator=(const GroupGuard&) = delete;
        GroupGuard(GroupGuard&&) = delete;
        GroupGuard& operator=(GroupGuard&&) = delete;

    private:
        SettingsManager& m_settings;  ///< 关联的配置管理器引用
    };

    /// 创建 RAII 分组守卫（推荐用于分组操作）
    GroupGuard groupGuard(const QString& group);

    // ---- 便捷方法: 串口配置 ----

    /// 保存串口配置到 "serial" 分组（先清除旧配置再写入）
    void saveSerialConfig(const QVariantMap& config);

    /// 加载串口配置，不存在时返回空 map
    QVariantMap loadSerialConfig() const;

    // ---- 便捷方法: 窗口几何 ----

    /// 保存窗口位置和大小（由 QWidget::saveGeometry() 生成）
    void saveWindowGeometry(const QByteArray& geometry);

    /// 加载窗口位置和大小，不存在时返回空 QByteArray
    QByteArray loadWindowGeometry() const;

    // ---- 便捷方法: 主题 ----

    /// 保存主题名称（如 "dark_terminal", "modern_dark", "light"）
    void saveTheme(const QString& themeName);

    /// 加载主题名称，默认 "dark_terminal"
    QString loadTheme() const;

    // ---- 便捷方法: 语言 ----

    /// 保存语言代码（"zh_CN" 或 "en"）
    void saveLanguage(const QString& langCode);

    /// 加载语言代码，默认 "zh_CN"
    QString loadLanguage() const;

    // ---- 统计计数器 ----

    /** @brief 获取配置读取总次数 @return 读取操作总次数 */
    quint64 totalReads() const;

    /** @brief 获取配置写入总次数 @return 写入操作总次数 */
    quint64 totalWrites() const;

    /** @brief 获取配置删除总次数 @return 删除操作总次数 */
    quint64 totalRemoves() const;

    /** @brief 获取配置操作错误总次数 @return 错误次数 */
    quint64 errorCount() const;

    /** @brief 重置所有配置管理统计计数器为零 */
    void resetSettingsStatistics();

private:
    /// 私有构造函数（单例模式）
    explicit SettingsManager(QObject* parent = nullptr);

    /// 析构函数 - 同步配置到磁盘，退出所有未关闭的分组
    ~SettingsManager() override;

    SettingsManager(const SettingsManager&) = delete;
    SettingsManager& operator=(const SettingsManager&) = delete;

    QSettings m_settings;              ///< Qt 配置存储引擎
    QStack<QString> m_groupStack;      ///< 分组嵌套栈

    // ---- 统计计数器 ----
    mutable quint64 m_totalReads = 0;      ///< 配置读取总次数
    quint64 m_totalWrites = 0;            ///< 配置写入总次数
    quint64 m_totalRemoves = 0;           ///< 配置删除总次数
    quint64 m_errorCount = 0;             ///< 配置操作错误总次数
};

#endif // SETTINGSMANAGER_H
