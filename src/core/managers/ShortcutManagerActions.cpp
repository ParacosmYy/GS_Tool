/**
 * @file ShortcutManagerActions.cpp
 * @brief ShortcutManager 冲突检测、使用统计与默认快捷键注册
 *
 * 从 ShortcutManager.cpp 拆分，包含:
 * - 按键冲突检测与自动解决
 * - 使用统计查询与重置
 * - EmbedDebug 默认快捷键集合注册
 */

#include "core/managers/ShortcutManager.h"
#include "utils/settings/SettingsManager.h"

#include <QShortcut>

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
