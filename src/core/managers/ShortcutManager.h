/**
 * @file ShortcutManager.h
 * @brief 统一键盘快捷键管理器 -- 集中注册和管理所有全局快捷键
 *
 * 支持功能:
 * - 集中注册所有全局快捷键
 * - 快捷键冲突检测
 * - 上下文感知(全局/终端/发送区)
 * - 快捷键可配置(为SettingsManager集成预留接口)
 *
 * 使用:
 *   auto& sm = ShortcutManager::instance();
 *   sm.registerShortcut("search", QKeySequence("Ctrl+F"), this, []() { ... });
 */

#ifndef SHORTCUTMANAGER_H
#define SHORTCUTMANAGER_H

#include <QObject>
#include <QShortcut>
#include <QKeySequence>
#include <QMap>
#include <QList>
#include <QHash>
#include <functional>

class QWidget;

/**
 * @brief 快捷键上下文枚举
 */
enum class ShortcutContext {
    Global,      ///< 全局(任何时刻有效)
    Terminal,    ///< 终端聚焦时有效
    SendArea     ///< 发送区聚焦时有效
};

/**
 * @brief 快捷键信息结构
 */
struct ShortcutInfo {
    QString id;                          ///< 唯一标识符(如 "search", "command_palette")
    QKeySequence keySequence;            ///< 按键序列
    QString description;                 ///< 中文描述
    ShortcutContext context;             ///< 上下文
    QShortcut* shortcut = nullptr;       ///< Qt快捷键对象
};

/**
 * @brief 统一键盘快捷键管理器
 *
 * 单例模式。集中管理所有快捷键的注册、冲突检测、上下文控制。
 * 所有快捷键通过ID注册，支持运行时修改按键序列。
 */
class ShortcutManager : public QObject {
    Q_OBJECT

public:
    /** @brief 获取单例实例 */
    static ShortcutManager& instance();

    /**
     * @brief 注册快捷键
     * @param id 唯一标识符
     * @param key 按键序列
     * @param parent 父Widget(用于创建QShortcut)
     * @param callback 回调函数
     * @param description 中文描述
     * @param context 快捷键上下文
     * @return true=注册成功, false=ID冲突或按键冲突
     */
    bool registerShortcut(const QString& id, const QKeySequence& key,
                          QWidget* parent, std::function<void()> callback,
                          const QString& description,
                          ShortcutContext context = ShortcutContext::Global);

    /**
     * @brief 注销快捷键
     * @param id 快捷键标识符
     */
    void unregisterShortcut(const QString& id);

    /**
     * @brief 修改快捷键绑定
     * @param id 快捷键标识符
     * @param newKey 新的按键序列
     * @return true=修改成功, false=ID不存在或新按键已被占用
     */
    bool rebind(const QString& id, const QKeySequence& newKey);

    /** @brief 获取所有已注册快捷键信息 */
    QList<ShortcutInfo> allShortcuts() const;

    // ---- 统计计数器 ----

    /** @brief 获取快捷键注册总次数（含成功和失败） @return 注册操作总次数 */
    quint64 totalRegistrations() const;

    /** @brief 获取快捷键触发总次数 @return 触发总次数 */
    quint64 totalTriggers() const;

    /** @brief 重置所有快捷键管理统计计数器为零 */
    void resetShortcutStatistics();

    /**
     * @brief 根据ID获取快捷键描述+按键文本(用于Tooltip)
     * @param id 快捷键标识符
     * @return 格式如 "打开搜索栏 (Ctrl+F)"，ID不存在返回空字符串
     */
    QString shortcutTooltip(const QString& id) const;

    /**
     * @brief 检查按键序列是否已被占用
     * @param key 要检查的按键序列
     * @param excludeId 排除的快捷键ID(用于rebind时排除自身)
     * @return true=已被占用
     */
    bool isKeyOccupied(const QKeySequence& key, const QString& excludeId = QString()) const;

signals:
    /** @brief 快捷键绑定变更信号 */
    void shortcutChanged(const QString& id, const QKeySequence& newKey);

private:
    explicit ShortcutManager(QObject* parent = nullptr);
    ~ShortcutManager() override;
    ShortcutManager(const ShortcutManager&) = delete;
    ShortcutManager& operator=(const ShortcutManager&) = delete;

    QMap<QString, ShortcutInfo> m_shortcuts;  ///< ID -> 快捷键信息

    // ---- 统计计数器 ----
    quint64 m_totalRegistrations = 0;  ///< 快捷键注册总次数
    quint64 m_totalTriggers = 0;       ///< 快捷键触发总次数
};

#endif // SHORTCUTMANAGER_H
