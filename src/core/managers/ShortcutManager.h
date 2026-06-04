/**
 * @file ShortcutManager.h
 * @brief 统一键盘快捷键管理器 -- 集中注册、上下文感知、冲突检测、持久化
 *
 * 功能: 上下文感知激活 / 冲突检测与自动解决 / 使用统计 / QSettings持久化 / 全局事件过滤
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

class QWidget; class QEvent;

/// @brief 快捷键上下文枚举
enum class ShortcutContext { Global, Terminal, SendArea };

/// @brief 快捷键信息结构
struct ShortcutInfo {
    QString id;                          ///< 唯一标识符
    QKeySequence keySequence, defaultKeySequence;
    QString description;                 ///< 中文描述
    ShortcutContext context;
    QShortcut* shortcut = nullptr;
    std::function<void()> callback;
    quint64 triggerCount = 0;
};

/** @brief 统一键盘快捷键管理器(单例) -- 集中管理注册、冲突检测、上下文控制和持久化 */
class ShortcutManager : public QObject {
    Q_OBJECT

public:
    static ShortcutManager& instance();

    bool registerShortcut(const QString& id, const QKeySequence& key,
                          QWidget* parent, std::function<void()> callback,
                          const QString& description,
                          ShortcutContext context = ShortcutContext::Global);
    void unregisterShortcut(const QString& id);
    bool rebind(const QString& id, const QKeySequence& newKey);
    QList<ShortcutInfo> allShortcuts() const;
    QList<ShortcutInfo> shortcutsByContext(ShortcutContext context) const;

    void setCurrentContext(ShortcutContext context);
    ShortcutContext currentContext() const;
    void registerContextWidget(QWidget* widget, ShortcutContext context);
    void unregisterContextWidget(QWidget* widget);

    bool isKeyOccupied(const QKeySequence& key, const QString& excludeId = QString()) const;
    QStringList findConflicts(const QKeySequence& key, const QString& excludeId = QString()) const;
    int resolveConflicts(const QString& preferredId, const QStringList& conflictIds);

    // ---- 使用统计 ----
    quint64 shortcutTriggerCount(const QString& id) const;
    quint64 totalRegistrations() const;
    quint64 totalTriggers() const;
    quint64 totalUnregistrations() const { return m_totalUnregistrations; }
    quint64 totalConflictsDetected() const { return m_totalConflictsDetected; }
    quint64 totalRebinds() const { return m_totalRebinds; }
    quint64 totalContextSwitches() const { return m_totalContextSwitches; }
    quint64 totalFailedRegistrations() const { return m_totalFailedRegistrations; }
    void resetShortcutStatistics();

    // ---- 持久化 ----
    void saveCustomBindings();
    void loadCustomBindings();
    void resetToDefault(const QString& id);
    void resetAllToDefaults();

    // ---- 工具 ----
    QString shortcutTooltip(const QString& id) const;
    QKeySequence shortcutKey(const QString& id) const;
    void registerDefaults(QWidget* mainWindow);

signals:
    void shortcutChanged(const QString& id, const QKeySequence& newKey);
    void conflictDetected(const QString& id1, const QString& id2);
    void shortcutTriggered(const QString& id);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    explicit ShortcutManager(QObject* parent = nullptr);
    ~ShortcutManager() override;
    ShortcutManager(const ShortcutManager&) = delete;
    ShortcutManager& operator=(const ShortcutManager&) = delete;
    bool isShortcutActiveInContext(const ShortcutInfo& info) const;
    void updateShortcutStates();

    QMap<QString, ShortcutInfo> m_shortcuts;
    QHash<QWidget*, ShortcutContext> m_contextMap;
    QSet<QString> m_customizedIds;
    ShortcutContext m_currentContext = ShortcutContext::Global;
    quint64 m_totalRegistrations = 0, m_totalTriggers = 0, m_totalUnregistrations = 0;
    quint64 m_totalConflictsDetected = 0, m_totalRebinds = 0;
    quint64 m_totalContextSwitches = 0, m_totalFailedRegistrations = 0;
    static constexpr const char* kSettingsGroup = "shortcuts";
};

#endif // SHORTCUTMANAGER_H
