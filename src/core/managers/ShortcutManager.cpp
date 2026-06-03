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

// ─── 查询 ────────────────────────────────────────────────

/** @brief 获取所有已注册快捷键 */
QList<ShortcutInfo> ShortcutManager::allShortcuts() const
{
    return m_shortcuts.values();
}

/** @brief 按上下文获取快捷键列表 @param context 上下文 */
QList<ShortcutInfo> ShortcutManager::shortcutsByContext(ShortcutContext context) const
{
    QList<ShortcutInfo> result;
    for (auto it = m_shortcuts.constBegin(); it != m_shortcuts.constEnd(); ++it) {
        if (it->context == context) result.append(it.value());
    }
    return result;
}

/** @brief 获取指定快捷键的按键序列 @param id 标识符 */
QKeySequence ShortcutManager::shortcutKey(const QString& id) const
{
    auto it = m_shortcuts.constFind(id);
    return (it != m_shortcuts.constEnd()) ? it->keySequence : QKeySequence();
}

/** @brief 获取Tooltip文本(格式: "描述 (Ctrl+F)") @param id 标识符 */
QString ShortcutManager::shortcutTooltip(const QString& id) const
{
    auto it = m_shortcuts.constFind(id);
    if (it == m_shortcuts.constEnd()) return {};
    return tr("%1 (%2)").arg(it->description,
                             it->keySequence.toString(QKeySequence::NativeText));
}

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

// ─── 冲突检测与解决 ─────────────────────────────────────

/** @brief 检查按键是否已被占用 @param key 按键 @param excludeId 排除ID */
bool ShortcutManager::isKeyOccupied(const QKeySequence& key, const QString& excludeId) const
{
    for (auto it = m_shortcuts.constBegin(); it != m_shortcuts.constEnd(); ++it) {
        if (it.key() == excludeId) continue;
        if (it->keySequence == key) return true;
    }
    return false;
}

/** @brief 查找所有冲突ID @param key 按键 @param excludeId 排除ID */
QStringList ShortcutManager::findConflicts(const QKeySequence& key,
                                            const QString& excludeId) const
{
    QStringList result;
    for (auto it = m_shortcuts.constBegin(); it != m_shortcuts.constEnd(); ++it) {
        if (it.key() == excludeId) continue;
        if (it->keySequence == key) result.append(it.key());
    }
    return result;
}

/**
 * @brief 自动解决冲突: 尝试Shift/Alt修饰键替代方案
 * @param preferredId 优先保留的ID
 * @param conflictIds 需要重绑定的冲突ID列表
 * @return 成功解决的冲突数
 */
int ShortcutManager::resolveConflicts(const QString& preferredId, const QStringList& conflictIds)
{
    int resolved = 0;
    for (const auto& id : conflictIds) {
        auto it = m_shortcuts.find(id);
        if (it == m_shortcuts.end()) continue;

        // 从默认键出发，尝试添加Shift或Alt修饰
        int combo = it->defaultKeySequence[0];
        int key = combo & ~Qt::KeyboardModifierMask;
        int mods = combo & Qt::KeyboardModifierMask;

        QKeySequence candidate;
        if ((mods & Qt::ShiftModifier) == 0) {
            candidate = QKeySequence((mods | Qt::ShiftModifier) | key);
        }
        if (candidate.isEmpty() || isKeyOccupied(candidate, id)) {
            candidate = QKeySequence((mods | Qt::AltModifier) | key);
        }
        if (isKeyOccupied(candidate, id)) continue;

        it->keySequence = candidate;
        if (it->shortcut) it->shortcut->setKey(candidate);
        m_customizedIds.insert(id);
        ++resolved;
        emit shortcutChanged(id, candidate);
    }
    if (resolved > 0) saveCustomBindings();
    return resolved;
}

// ─── 使用统计 ────────────────────────────────────────────

/** @brief 获取指定快捷键触发次数 @param id 标识符 */
quint64 ShortcutManager::shortcutTriggerCount(const QString& id) const
{
    auto it = m_shortcuts.constFind(id);
    return (it != m_shortcuts.constEnd()) ? it->triggerCount : 0;
}

/** @brief 注册总次数 */
quint64 ShortcutManager::totalRegistrations() const { return m_totalRegistrations; }

/** @brief 触发总次数 */
quint64 ShortcutManager::totalTriggers() const { return m_totalTriggers; }

/** @brief 重置所有统计计数器 */
void ShortcutManager::resetShortcutStatistics()
{
    m_totalRegistrations = 0;
    m_totalTriggers = 0;
    for (auto it = m_shortcuts.begin(); it != m_shortcuts.end(); ++it)
        it->triggerCount = 0;
}

// ─── 持久化 ─────────────────────────────────────────────

/** @brief 将自定义绑定保存到QSettings */
void ShortcutManager::saveCustomBindings()
{
    auto& settings = SettingsManager::instance();
    auto guard = settings.groupGuard(kSettingsGroup);
    for (const auto& id : m_customizedIds) {
        auto it = m_shortcuts.constFind(id);
        if (it != m_shortcuts.constEnd())
            settings.set(id, it->keySequence.toString());
    }
    settings.sync();
}

/** @brief 从QSettings加载自定义绑定并应用 */
void ShortcutManager::loadCustomBindings()
{
    auto& settings = SettingsManager::instance();
    auto guard = settings.groupGuard(kSettingsGroup);
    for (const auto& id : settings.groupKeys()) {
        auto it = m_shortcuts.find(id);
        if (it == m_shortcuts.end()) continue;

        QKeySequence loaded(settings.get(id).toString());
        if (loaded.isEmpty()) continue;
        // 加载时检查冲突
        if (isKeyOccupied(loaded, id)) continue;

        it->keySequence = loaded;
        if (it->shortcut) it->shortcut->setKey(loaded);
        m_customizedIds.insert(id);
    }
}

/** @brief 恢复指定快捷键为默认绑定 @param id 标识符 */
void ShortcutManager::resetToDefault(const QString& id)
{
    auto it = m_shortcuts.find(id);
    if (it == m_shortcuts.end()) return;
    if (isKeyOccupied(it->defaultKeySequence, id)) return;

    it->keySequence = it->defaultKeySequence;
    if (it->shortcut) it->shortcut->setKey(it->defaultKeySequence);
    m_customizedIds.remove(id);

    auto& settings = SettingsManager::instance();
    auto guard = settings.groupGuard(kSettingsGroup);
    settings.remove(id);
    settings.sync();

    emit shortcutChanged(id, it->defaultKeySequence);
}

/** @brief 恢复所有快捷键为默认绑定 */
void ShortcutManager::resetAllToDefaults()
{
    for (auto it = m_shortcuts.begin(); it != m_shortcuts.end(); ++it) {
        it->keySequence = it->defaultKeySequence;
        if (it->shortcut) it->shortcut->setKey(it->defaultKeySequence);
    }
    m_customizedIds.clear();
    SettingsManager::instance().removeGroup(kSettingsGroup);
    SettingsManager::instance().sync();
}

// ─── 默认快捷键注册 ──────────────────────────────────────

/**
 * @brief 注册EmbedDebug标准快捷键集合
 * Ctrl+F搜索 Ctrl+P命令面板 Ctrl+Shift+R录制
 * Ctrl+Enter发送 Ctrl+L清空终端 Ctrl+S保存
 * Ctrl+O打开 Ctrl+N新建连接 Ctrl+W关闭标签
 * @param mainWindow 主窗口(作为QShortcut的parent)
 */
void ShortcutManager::registerDefaults(QWidget* mainWindow)
{
    // ---- 全局(Global) ----
    registerShortcut("search.find", QKeySequence("Ctrl+F"),
        mainWindow, nullptr, tr("搜索"), ShortcutContext::Global);
    registerShortcut("nav.commandPalette", QKeySequence("Ctrl+P"),
        mainWindow, nullptr, tr("命令面板"), ShortcutContext::Global);
    registerShortcut("project.save", QKeySequence("Ctrl+S"),
        mainWindow, nullptr, tr("保存工程"), ShortcutContext::Global);
    registerShortcut("project.open", QKeySequence("Ctrl+O"),
        mainWindow, nullptr, tr("打开工程"), ShortcutContext::Global);
    registerShortcut("connection.new", QKeySequence("Ctrl+N"),
        mainWindow, nullptr, tr("新建连接"), ShortcutContext::Global);
    registerShortcut("tab.close", QKeySequence("Ctrl+W"),
        mainWindow, nullptr, tr("关闭标签页"), ShortcutContext::Global);

    // ---- 终端(Terminal) ----
    registerShortcut("terminal.clear", QKeySequence("Ctrl+L"),
        mainWindow, nullptr, tr("清空终端"), ShortcutContext::Terminal);
    registerShortcut("script.recordToggle", QKeySequence("Ctrl+Shift+R"),
        mainWindow, nullptr, tr("录制脚本"), ShortcutContext::Terminal);

    // ---- 发送区(SendArea) ----
    registerShortcut("send.execute", QKeySequence("Ctrl+Enter"),
        mainWindow, nullptr, tr("发送数据"), ShortcutContext::SendArea);

    // 加载用户自定义绑定(覆盖默认值)
    loadCustomBindings();
}
