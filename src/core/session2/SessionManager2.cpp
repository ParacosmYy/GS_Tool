#include "core/session2/SessionManager2.h"
#include <QJsonDocument>
#include <QFile>
SessionManager::SessionManager(QObject *parent) : QObject(parent) {}
SessionManager::~SessionManager() = default;
QString SessionManager::createSession(const QString &name) {
    QString id = QString("sess_%1").arg(++m_counter);
    Session s; s.id = id; s.name = name; s.created = QDateTime::currentMSecsSinceEpoch(); s.modified = s.created;
    m_sessions[id] = s; emit sessionCreated(id); return id;
}
void SessionManager::deleteSession(const QString &id) { m_sessions.remove(id); if (m_currentId == id) m_currentId.clear(); emit sessionDeleted(id); }
void SessionManager::switchSession(const QString &id) { if (m_sessions.contains(id)) { m_currentId = id; emit sessionSwitched(id); } }
void SessionManager::saveState(const QString &key, const QVariant &val) { if (m_sessions.contains(m_currentId)) { m_sessions[m_currentId].state[key] = val; m_sessions[m_currentId].modified = QDateTime::currentMSecsSinceEpoch(); emit stateSaved(key); } }
QVariant SessionManager::state(const QString &key) const { return m_sessions.value(m_currentId).state.value(key); }
void SessionManager::saveAll(const QVariantMap &m) { if (m_sessions.contains(m_currentId)) { m_sessions[m_currentId].state = m; m_sessions[m_currentId].modified = QDateTime::currentMSecsSinceEpoch(); } }
QVariantMap SessionManager::loadAll() const { return m_sessions.value(m_currentId).state; }
SessionManager::Session SessionManager::currentSession() const { return m_sessions.value(m_currentId); }
QList<SessionManager::Session> SessionManager::allSessions() const { return m_sessions.values(); }
QStringList SessionManager::sessionNames() const { QStringList n; for (const auto &s : m_sessions) n << s.name; return n; }
void SessionManager::exportSession(const QString &id, const QString &path) { Q_UNUSED(id); Q_UNUSED(path); }
QString SessionManager::importSession(const QString &path) { Q_UNUSED(path); return {}; }
