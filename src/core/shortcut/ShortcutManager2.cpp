/**
 * @file ShortcutManager2.cpp
 * @brief 快捷键管理器v2实现 — 注册/重绑定/冲突检测/分类查询
 */
#include "core/shortcut/ShortcutManager2.h"

/** @brief 构造函数 @param parent 父对象 */
ShortcutManager::ShortcutManager(QObject *parent) : QObject(parent) {}
/** @brief 析构函数 */
ShortcutManager::~ShortcutManager() = default;

/** @brief 注册快捷键 @param id 唯一标识 @param label 显示标签 @param key 默认键序列 @param cat 分类名称 */
void ShortcutManager::registerShortcut(const QString &id, const QString &label, const QKeySequence &key, const QString &cat) { m_shortcuts[id] = {id, label, key, key, cat}; emit shortcutRegistered(id); }
/** @brief 注销快捷键 @param id 唯一标识 */
void ShortcutManager::unregisterShortcut(const QString &id) { m_shortcuts.remove(id); }
/** @brief 重新绑定快捷键 @param id 唯一标识 @param key 新的键序列 */
void ShortcutManager::rebind(const QString &id, const QKeySequence &key) { auto it = m_shortcuts.find(id); if (it != m_shortcuts.end()) { it->currentKey = key; emit shortcutRebound(id, key); } }
/** @brief 重置指定快捷键为默认绑定 @param id 唯一标识 */
void ShortcutManager::resetToDefault(const QString &id) { auto it = m_shortcuts.find(id); if (it != m_shortcuts.end()) it->currentKey = it->defaultKey; }
/** @brief 重置所有快捷键为默认绑定 */
void ShortcutManager::resetAll() { for (auto it = m_shortcuts.begin(); it != m_shortcuts.end(); ++it) it->currentKey = it->defaultKey; }
/** @brief 获取快捷键当前绑定 @param id 唯一标识 @return 当前键序列 */
QKeySequence ShortcutManager::shortcut(const QString &id) const { return m_shortcuts.value(id).currentKey; }
/** @brief 获取快捷键显示标签 @param id 唯一标识 @return 标签文本 */
QString ShortcutManager::shortcutLabel(const QString &id) const { return m_shortcuts.value(id).label; }
/** @brief 获取所有已注册快捷键 @return 快捷键条目列表 */
QList<ShortcutManager::ShortcutEntry> ShortcutManager::allShortcuts() const { return m_shortcuts.values(); }
/** @brief 按分类查询快捷键 @param cat 分类名称 @return 该分类下的快捷键列表 */
QList<ShortcutManager::ShortcutEntry> ShortcutManager::shortcutsByCategory(const QString &cat) const { QList<ShortcutEntry> r; for (const auto &s : m_shortcuts) if (s.category == cat) r.append(s); return r; }
/** @brief 获取所有分类名称 @return 分类列表 */
QStringList ShortcutManager::categories() const { QStringList c; for (const auto &s : m_shortcuts) if (!c.contains(s.category)) c.append(s.category); return c; }
/** @brief 检测键序列是否与已有快捷键冲突 @param key 待检测键序列 @param out 输出冲突的快捷键ID @return 存在冲突返回true */
bool ShortcutManager::hasConflict(const QKeySequence &key, QString *out) const { for (const auto &s : m_shortcuts) if (s.currentKey == key && !key.isEmpty()) { if (out) *out = s.id; return true; } return false; }
