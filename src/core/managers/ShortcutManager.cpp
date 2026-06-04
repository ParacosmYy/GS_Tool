/**
 * @file ShortcutManager.cpp
 * @brief 统一键盘快捷键管理器实现 -- 上下文感知/冲突检测/持久化/事件过滤
 *
 * 单例模式，集中管理所有快捷键的注册、冲突检测、上下文控制和持久化。
 * 通过全局事件过滤器拦截按键事件，基于当前焦点上下文路由到正确处理函数。
 */

#include "core/managers/ShortcutManager.h"
#include "utils/settings/SettingsManager.h"

#include <QWidget>
#include <QEvent>
#include <QKeyEvent>
#include <QShortcut>
#include <QHash>
#include <QCoreApplication>

// ─── 单例 ────────────────────────────────────────────────

/** @brief 获取单例实例 */
ShortcutManager& ShortcutManager::instance()
{
    static ShortcutManager s_instance;
    return s_instance;
}

/** @brief 构造: 安装全局事件过滤器 @param parent 父对象 */
ShortcutManager::ShortcutManager(QObject* parent) : QObject(parent)
{
    QCoreApplication::instance()->installEventFilter(this);
}

/** @brief 析构: 移除事件过滤器 */
ShortcutManager::~ShortcutManager()
{
    QCoreApplication::instance()->removeEventFilter(this);
}

// ─── 注册 ────────────────────────────────────────────────

/**
 * @brief 注册快捷键
 * @param id 唯一标识符
 * @param key 按键序列
 * @param parent 父Widget
 * @param callback 回调函数
 * @param description 中文描述
 * @param context 上下文
 * @return true=成功, false=ID或按键冲突
 */
bool ShortcutManager::registerShortcut(const QString& id, const QKeySequence& key,
                                        QWidget* parent, std::function<void()> callback,
                                        const QString& description, ShortcutContext context)
{
    ++m_totalRegistrations;

    // ID冲突检查
    if (m_shortcuts.contains(id)) {
        return false;
    }
    // 按键冲突检查
    if (isKeyOccupied(key)) {
        return false;
    }

    auto* shortcut = new QShortcut(key, parent);
    shortcut->setContext(Qt::ApplicationShortcut);

    // 根据当前上下文设置初始启用状态
    ShortcutInfo tempInfo;
    tempInfo.context = context;
    shortcut->setEnabled(isShortcutActiveInContext(tempInfo));

    // 构造存储信息
    ShortcutInfo info;
    info.id = id;
    info.keySequence = key;
    info.defaultKeySequence = key;
    info.description = description;
    info.context = context;
    info.shortcut = shortcut;
    info.callback = callback;
    info.triggerCount = 0;

    // 连接回调(通过id间接引用，避免悬空捕获)
    connect(shortcut, &QShortcut::activated, this, [this, id]() {
        auto it = m_shortcuts.find(id);
        if (it == m_shortcuts.end()) return;
        ++it->triggerCount;
        ++m_totalTriggers;
        if (it->callback) it->callback();
        emit shortcutTriggered(id);
    });

    m_shortcuts.insert(id, info);
    return true;
}

// ─── 注销 ────────────────────────────────────────────────

/** @brief 注销快捷键 @param id 标识符 */
void ShortcutManager::unregisterShortcut(const QString& id)
{
    auto it = m_shortcuts.find(id);
    if (it == m_shortcuts.end()) return;
    if (it->shortcut) delete it->shortcut;
    m_shortcuts.erase(it);
    m_customizedIds.remove(id);
}

// ─── 修改绑定 ────────────────────────────────────────────

/** @brief 修改快捷键绑定并持久化 @param id 标识符 @param newKey 新序列 @return true=成功 */
bool ShortcutManager::rebind(const QString& id, const QKeySequence& newKey)
{
    auto it = m_shortcuts.find(id);
    if (it == m_shortcuts.end()) return false;

    // 检查新按键冲突(排除自身)
    if (isKeyOccupied(newKey, id)) {
        auto conflicts = findConflicts(newKey, id);
        for (const auto& cid : conflicts) emit conflictDetected(id, cid);
        return false;
    }

    it->keySequence = newKey;
    if (it->shortcut) it->shortcut->setKey(newKey);

    m_customizedIds.insert(id);
    saveCustomBindings();
    emit shortcutChanged(id, newKey);
    return true;
}

// ─── 查询见 ShortcutManagerQuery.cpp ────────────────────

// ─── 上下文感知 ──────────────────────────────────────────

/** @brief 设置当前上下文并更新快捷键启用状态 @param context 上下文 */
void ShortcutManager::setCurrentContext(ShortcutContext context)
{
    if (m_currentContext == context) return;
    m_currentContext = context;
    updateShortcutStates();
}

/** @brief 获取当前上下文 */
ShortcutContext ShortcutManager::currentContext() const { return m_currentContext; }

/** @brief 注册Widget到上下文映射 @param widget 控件 @param context 上下文 */
void ShortcutManager::registerContextWidget(QWidget* widget, ShortcutContext context)
{
    if (widget) m_contextMap.insert(widget, context);
}

/** @brief 移除上下文映射 @param widget 控件 */
void ShortcutManager::unregisterContextWidget(QWidget* widget) { m_contextMap.remove(widget); }

/**
 * @brief 全局事件过滤器 - 焦点进入时自动切换上下文
 * @return 始终返回false(不消费事件)
 */
bool ShortcutManager::eventFilter(QObject* watched, QEvent* event)
{
    if (event->type() == QEvent::FocusIn) {
        auto* widget = qobject_cast<QWidget*>(watched);
        if (widget) {
            // 向上遍历父控件查找已注册的上下文
            QWidget* cur = widget;
            while (cur) {
                auto it = m_contextMap.constFind(cur);
                if (it != m_contextMap.constEnd()) {
                    setCurrentContext(it.value());
                    return false;
                }
                cur = cur->parentWidget();
            }
            // 未找到映射则回退到全局
            setCurrentContext(ShortcutContext::Global);
        }
    }
    return false;
}

/** @brief 检查快捷键是否在当前上下文中可激活 @param info 快捷键信息 */
bool ShortcutManager::isShortcutActiveInContext(const ShortcutInfo& info) const
{
    // Global始终激活; 其他仅匹配时激活
    return (info.context == ShortcutContext::Global || info.context == m_currentContext);
}

/** @brief 更新所有QShortcut启用状态 */
void ShortcutManager::updateShortcutStates()
{
    for (auto it = m_shortcuts.begin(); it != m_shortcuts.end(); ++it) {
        if (it->shortcut) it->shortcut->setEnabled(isShortcutActiveInContext(it.value()));
    }
}

// ─── 冲突检测/统计/registerDefaults 见 ShortcutManagerActions.cpp ──
// ─── 持久化(save/load/reset)见 ShortcutManagerPersist.cpp ──

// registerDefaults() 见 ShortcutManagerActions.cpp
