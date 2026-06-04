/**
 * @file ShortcutManagerPersist.cpp
 * @brief 快捷键管理器持久化方法实现
 *
 * 从 ShortcutManager.cpp 拆分而来，包含快捷键绑定的
 * 保存、加载和恢复默认方法。
 */

#include "core/managers/ShortcutManager.h"
#include "utils/settings/SettingsManager.h"

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
        /* 加载时检查冲突 */
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
