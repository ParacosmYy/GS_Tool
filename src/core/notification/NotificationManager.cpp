#include "core/notification/NotificationManager.h"
#include <QDateTime>

NotificationManager::NotificationManager(QObject *parent) : QObject(parent) {}
NotificationManager::~NotificationManager() = default;

QString NotificationManager::notify(const QString &title, const QString &msg, Priority prio) {
    QString id = QString("notif_%1").arg(++m_counter);
    Notification n;
    n.id = id; n.title = title; n.message = msg; n.priority = prio;
    n.timestamp = QDateTime::currentMSecsSinceEpoch(); n.acknowledged = false;
    m_active[id] = n;
    m_history.prepend(n);
    pruneHistory();
    emit notificationAdded(n);
    emit unreadCountChanged(unreadCount());
    return id;
}

void NotificationManager::acknowledge(const QString &id) {
    auto it = m_active.find(id);
    if (it != m_active.end()) { it->acknowledged = true; emit notificationAcknowledged(id); emit unreadCountChanged(unreadCount()); }
}

void NotificationManager::dismiss(const QString &id) {
    m_active.remove(id); emit notificationDismissed(id); emit unreadCountChanged(unreadCount());
}

void NotificationManager::clearAll() { m_active.clear(); m_history.clear(); emit unreadCountChanged(0); }

QList<NotificationManager::Notification> NotificationManager::activeNotifications() const { return m_active.values(); }
QList<NotificationManager::Notification> NotificationManager::allNotifications() const { return m_history; }

int NotificationManager::unreadCount() const {
    int c = 0;
    for (auto it = m_active.constBegin(); it != m_active.constEnd(); ++it)
        if (!it->acknowledged) c++;
    return c;
}

void NotificationManager::setMaxHistory(int m) { m_maxHistory = m; pruneHistory(); }

void NotificationManager::pruneHistory() { while (m_history.size() > m_maxHistory) m_history.removeLast(); }
