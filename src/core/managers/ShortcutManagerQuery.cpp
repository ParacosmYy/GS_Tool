/**
 * @file ShortcutManagerQuery.cpp
 * @brief ShortcutManager 快捷键查询方法实现
 *
 * 从 ShortcutManager.cpp 拆分，包含:
 * - 全量快捷键列表查询
 * - 按上下文筛选快捷键
 * - 单个快捷键按键序列查询
 * - Tooltip格式化文本生成
 */

#include "core/managers/ShortcutManager.h"

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
