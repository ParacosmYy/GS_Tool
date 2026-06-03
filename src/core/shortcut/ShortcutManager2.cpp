#include "core/shortcut/ShortcutManager2.h"
ShortcutManager::ShortcutManager(QObject *parent) : QObject(parent) {}
ShortcutManager::~ShortcutManager() = default;
void ShortcutManager::registerShortcut(const QString &id, const QString &label, const QKeySequence &key, const QString &cat) { m_shortcuts[id] = {id, label, key, key, cat}; emit shortcutRegistered(id); }
void ShortcutManager::unregisterShortcut(const QString &id) { m_shortcuts.remove(id); }
void ShortcutManager::rebind(const QString &id, const QKeySequence &key) { auto it = m_shortcuts.find(id); if (it != m_shortcuts.end()) { it->currentKey = key; emit shortcutRebound(id, key); } }
void ShortcutManager::resetToDefault(const QString &id) { auto it = m_shortcuts.find(id); if (it != m_shortcuts.end()) it->currentKey = it->defaultKey; }
void ShortcutManager::resetAll() { for (auto it = m_shortcuts.begin(); it != m_shortcuts.end(); ++it) it->currentKey = it->defaultKey; }
QKeySequence ShortcutManager::shortcut(const QString &id) const { return m_shortcuts.value(id).currentKey; }
QString ShortcutManager::shortcutLabel(const QString &id) const { return m_shortcuts.value(id).label; }
QList<ShortcutManager::ShortcutEntry> ShortcutManager::allShortcuts() const { return m_shortcuts.values(); }
QList<ShortcutManager::ShortcutEntry> ShortcutManager::shortcutsByCategory(const QString &cat) const { QList<ShortcutEntry> r; for (const auto &s : m_shortcuts) if (s.category == cat) r.append(s); return r; }
QStringList ShortcutManager::categories() const { QStringList c; for (const auto &s : m_shortcuts) if (!c.contains(s.category)) c.append(s.category); return c; }
bool ShortcutManager::hasConflict(const QKeySequence &key, QString *out) const { for (const auto &s : m_shortcuts) if (s.currentKey == key && !key.isEmpty()) { if (out) *out = s.id; return true; } return false; }
