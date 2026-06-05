/** @file SettingsManager.h @brief 配置管理器 - 单例，封装QSettings提供类型安全的配置读写。支持嵌套分组/RAII守卫/串口配置/窗口几何/主题/语言 */
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
 * @brief 配置管理器 - 单例，封装QSettings提供类型安全的配置读写
 *
 * 使用: instance().set("key", value) / instance().get("key", default)
 * RAII守卫: auto guard = instance().groupGuard("serial"); // 析构自动endGroup
 * 线程安全: QSettings本身线程安全，但beginGroup/endGroup不支持多线程并发
 */
class SettingsManager : public QObject {
    Q_OBJECT

public:
    static SettingsManager& instance(); ///< 获取全局唯一实例
    // ---- 基本读写接口 ----
    QVariant get(const QString& key, const QVariant& defaultValue = QVariant()) const; ///< 读取(支持"/"嵌套键)
    void set(const QString& key, const QVariant& value); ///< 写入
    void remove(const QString& key);                      ///< 删除单个配置项
    bool contains(const QString& key) const;              ///< 检查是否存在
    void sync();                                          ///< 同步写入磁盘(析构自动调用)
    // ---- 分组操作接口 ----
    void beginGroup(const QString& group);                ///< 进入分组(支持嵌套)
    void endGroup();                                      ///< 退出当前分组
    QString currentGroup() const;                         ///< 当前分组路径
    void removeGroup(const QString& group);               ///< 删除整个分组
    bool containsGroup(const QString& group) const;       ///< 检查分组是否存在且有内容
    QStringList groupKeys(const QString& group = QString()) const;   ///< 分组下所有键名
    QStringList childGroups(const QString& group = QString()) const; ///< 分组下所有子分组名
    // ---- RAII 分组守卫 ----
    /** @brief RAII分组守卫 - 构造beginGroup，析构endGroup。禁止拷贝/移动 */
    class GroupGuard {
    public:
        GroupGuard(SettingsManager& settings, const QString& group) : m_settings(settings) { m_settings.beginGroup(group); }
        ~GroupGuard() { m_settings.endGroup(); }
        GroupGuard(const GroupGuard&) = delete;
        GroupGuard& operator=(const GroupGuard&) = delete;
        GroupGuard(GroupGuard&&) = delete;
        GroupGuard& operator=(GroupGuard&&) = delete;
    private:
        SettingsManager& m_settings;
    };
    GroupGuard groupGuard(const QString& group); ///< 创建RAII分组守卫(推荐)
    // ---- 便捷方法: 串口配置 ----
    void saveSerialConfig(const QVariantMap& config); ///< 保存串口配置到"serial"分组
    QVariantMap loadSerialConfig() const;             ///< 加载串口配置
    // ---- 便捷方法: 窗口几何 ----
    void saveWindowGeometry(const QByteArray& geometry); ///< 保存窗口位置大小
    QByteArray loadWindowGeometry() const;               ///< 加载窗口位置大小
    // ---- 便捷方法: 主题 ----
    void saveTheme(const QString& themeName); ///< 保存主题名称
    QString loadTheme() const;                ///< 加载主题名称(默认"dark_terminal")
    // ---- 便捷方法: 语言 ----
    void saveLanguage(const QString& langCode); ///< 保存语言代码
    QString loadLanguage() const;                ///< 加载语言代码(默认"zh_CN")
    // ---- 统计计数器 ----
    quint64 totalReads() const;    ///< 配置读取总次数
    quint64 totalWrites() const;   ///< 配置写入总次数
    quint64 totalRemoves() const;  ///< 配置删除总次数
    quint64 errorCount() const;    ///< 配置操作错误总次数
    quint64 totalSyncs() const { return m_totalSyncs; } ///< 配置同步到磁盘总次数
    quint64 totalGroupEnters() const { return m_totalGroupEnters; } ///< 累计进入分组次数
    quint64 totalGroupExits() const { return m_totalGroupExits; } ///< 累计退出分组次数
    quint64 totalContainsChecks() const { return m_totalContainsChecks; } ///< 累计contains检查次数
    void resetSettingsStatistics();///< 重置所有统计计数器

private:
    explicit SettingsManager(QObject* parent = nullptr); ///< 私有构造(单例)
    ~SettingsManager() override;                         ///< 同步配置到磁盘，退出所有未关闭分组
    SettingsManager(const SettingsManager&) = delete;
    SettingsManager& operator=(const SettingsManager&) = delete;
    mutable QSettings m_settings;      ///< Qt配置存储引擎(const方法需beginGroup/endGroup)
    mutable QStack<QString> m_groupStack; ///< 分组嵌套栈
    // ---- 统计计数器 ----
    mutable quint64 m_totalReads = 0;      ///< 配置读取总次数
    quint64 m_totalWrites = 0;            ///< 配置写入总次数
    quint64 m_totalRemoves = 0;           ///< 配置删除总次数
    quint64 m_errorCount = 0;             ///< 配置操作错误总次数
    quint64 m_totalSyncs = 0;             ///< 配置同步到磁盘总次数
    quint64 m_totalGroupEnters = 0;       ///< 累计进入分组次数
    quint64 m_totalGroupExits = 0;        ///< 累计退出分组次数
    mutable quint64 m_totalContainsChecks = 0; ///< 累计contains检查次数
};

#endif // SETTINGSMANAGER_H
