/**
 * @file ShortcutManager.h
 * @brief 统一键盘快捷键管理器 -- 集中注册、上下文感知、冲突检测、持久化
 *
 * 功能: 上下文感知激活 / 冲突检测与自动解决 / 使用统计 / QSettings持久化 / 全局事件过滤
 * 用法: ShortcutManager::instance().registerShortcut("id", QKeySequence("Ctrl+F"), this, [](){...});
 */

#ifndef SHORTCUTMANAGER_H
#define SHORTCUTMANAGER_H

#include <QObject>
#include <QShortcut>
#include <QKeySequence>
#include <QMap>
#include <QList>
#include <QHash>
#include <QSet>
#include <functional>

class QWidget;
class QEvent;

/// @brief 快捷键上下文枚举
enum class ShortcutContext {
    Global,      ///< 全局(任何时刻有效)
    Terminal,    ///< 终端聚焦时有效
    SendArea     ///< 发送区聚焦时有效
};

/// @brief 快捷键信息结构
struct ShortcutInfo {
    QString id;                          ///< 唯一标识符(如 "search.find")
    QKeySequence keySequence;            ///< 当前按键序列
    QKeySequence defaultKeySequence;     ///< 默认按键序列(用于重置)
    QString description;                 ///< 中文描述
    ShortcutContext context;             ///< 上下文
    QShortcut* shortcut = nullptr;       ///< Qt快捷键对象
    std::function<void()> callback;      ///< 激活回调函数
    quint64 triggerCount = 0;            ///< 该快捷键被触发的次数
};

/**
 * @brief 统一键盘快捷键管理器(单例)
 *
 * 集中管理所有快捷键的注册、冲突检测、上下文控制和持久化。
 * 通过全局事件过滤器实现上下文感知的快捷键路由。
 */
class ShortcutManager : public QObject {
    Q_OBJECT

public:
    /** @brief 获取单例实例 */
    static ShortcutManager& instance();

    /// @brief 注册快捷键 @return true=成功, false=ID或按键冲突
    bool registerShortcut(const QString& id, const QKeySequence& key,
                          QWidget* parent, std::function<void()> callback,
                          const QString& description,
                          ShortcutContext context = ShortcutContext::Global);
    /// @brief 注销快捷键
    void unregisterShortcut(const QString& id);
    /// @brief 修改快捷键绑定(同时持久化) @return true=成功
    bool rebind(const QString& id, const QKeySequence& newKey);
    /// @brief 获取所有已注册快捷键
    QList<ShortcutInfo> allShortcuts() const;
    /// @brief 按上下文获取快捷键列表
    QList<ShortcutInfo> shortcutsByContext(ShortcutContext context) const;

    // ---- 上下文感知 ----
    /// @brief 设置当前激活的上下文(焦点变化时调用)
    void setCurrentContext(ShortcutContext context);
    /// @brief 获取当前上下文
    ShortcutContext currentContext() const;
    /// @brief 注册Widget到上下文的映射(焦点进入时自动切换上下文)
    void registerContextWidget(QWidget* widget, ShortcutContext context);
    /// @brief 移除上下文映射
    void unregisterContextWidget(QWidget* widget);

    // ---- 冲突检测与解决 ----
    /// @brief 检查按键是否已被占用 @param excludeId 排除自身ID
    bool isKeyOccupied(const QKeySequence& key, const QString& excludeId = QString()) const;
    /// @brief 查找占用指定按键的所有快捷键ID
    QStringList findConflicts(const QKeySequence& key, const QString& excludeId = QString()) const;
    /// @brief 自动解决冲突: 将冲突方重绑定到替代序列 @return 成功解决的冲突数
    int resolveConflicts(const QString& preferredId, const QStringList& conflictIds);

    // ---- 使用统计 ----
    /// @brief 获取指定快捷键触发次数
    quint64 shortcutTriggerCount(const QString& id) const;
    /// @brief 注册总次数(含失败)
    quint64 totalRegistrations() const;
    /// @brief 触发总次数
    quint64 totalTriggers() const;
    /// @brief 注销总次数
    quint64 totalUnregistrations() const { return m_totalUnregistrations; }
    /// @brief 冲突检测总次数(按键已被占用时触发)
    quint64 totalConflictsDetected() const { return m_totalConflictsDetected; }
    /// @brief 获取重绑定总次数(成功修改按键序列)
    quint64 totalRebinds() const { return m_totalRebinds; }
    /// @brief 获取上下文切换总次数(焦点变化触发)
    quint64 totalContextSwitches() const { return m_totalContextSwitches; }
    /// @brief 获取注册失败总次数(ID冲突+按键冲突)
    quint64 totalFailedRegistrations() const { return m_totalFailedRegistrations; }
    /// @brief 重置所有统计计数器
    void resetShortcutStatistics();

    // ---- 持久化 ----
    /// @brief 保存自定义绑定到QSettings
    void saveCustomBindings();
    /// @brief 从QSettings加载自定义绑定
    void loadCustomBindings();
    /// @brief 恢复指定快捷键为默认绑定
    void resetToDefault(const QString& id);
    /// @brief 恢复所有快捷键为默认绑定
    void resetAllToDefaults();

    // ---- 工具方法 ----
    /// @brief 获取Tooltip文本(格式: "描述 (Ctrl+F)")
    QString shortcutTooltip(const QString& id) const;
    /// @brief 获取指定快捷键的当前按键序列
    QKeySequence shortcutKey(const QString& id) const;
    /// @brief 注册EmbedDebug默认快捷键集合(Ctrl+F/P/S/O/N/W/L/Ctrl+Shift+R/Ctrl+Enter)
    void registerDefaults(QWidget* mainWindow);

signals:
    /// @brief 快捷键绑定变更
    void shortcutChanged(const QString& id, const QKeySequence& newKey);
    /// @brief 检测到冲突
    void conflictDetected(const QString& id1, const QString& id2);
    /// @brief 快捷键被触发
    void shortcutTriggered(const QString& id);

protected:
    /// @brief 全局事件过滤器 - 焦点变化时自动切换上下文
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    explicit ShortcutManager(QObject* parent = nullptr);
    ~ShortcutManager() override;
    ShortcutManager(const ShortcutManager&) = delete;
    ShortcutManager& operator=(const ShortcutManager&) = delete;

    /// @brief 检查快捷键在当前上下文中是否激活
    bool isShortcutActiveInContext(const ShortcutInfo& info) const;
    /// @brief 根据当前上下文更新所有QShortcut启用状态
    void updateShortcutStates();

    QMap<QString, ShortcutInfo> m_shortcuts;        ///< ID -> 快捷键信息
    QHash<QWidget*, ShortcutContext> m_contextMap;  ///< Widget -> 上下文映射
    QSet<QString> m_customizedIds;                  ///< 用户自定义过的ID集合
    ShortcutContext m_currentContext = ShortcutContext::Global; ///< 当前上下文

    quint64 m_totalRegistrations = 0;    ///< 注册总次数
    quint64 m_totalTriggers = 0;         ///< 触发总次数
    quint64 m_totalUnregistrations = 0;  ///< 注销总次数
    quint64 m_totalConflictsDetected = 0;///< 冲突检测总次数(按键已被占用)
    quint64 m_totalRebinds = 0;          ///< 重绑定总次数(成功修改按键序列)
    quint64 m_totalContextSwitches = 0;  ///< 上下文切换总次数(焦点变化触发)
    quint64 m_totalFailedRegistrations = 0; ///< 注册失败总次数(ID冲突+按键冲突)

    static constexpr const char* kSettingsGroup = "shortcuts"; ///< QSettings分组名
};

#endif // SHORTCUTMANAGER_H
