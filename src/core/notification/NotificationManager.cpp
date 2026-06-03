/**
 * @file NotificationManager.cpp
 * @brief 通知管理器实现
 * @since score-132
 */
#include "core/notification/NotificationManager.h"
#include <QDateTime>

NotificationManager::NotificationManager(QObject *parent) : QObject(parent) {
    m_autoDismissTimer.setInterval(1000);
    connect(&m_autoDismissTimer, &QTimer::timeout, this, &NotificationManager::autoDismiss);
    m_autoDismissTimer.start();
}
NotificationManager::~NotificationManager() = default;
NotificationManager &NotificationManager::instance() { static NotificationManager inst; return inst; }

QString NotificationManager::showNotification(const QString &title, const QString &message, NotificationLevel level, int durationMs) {
    Notification n;
    n.id = generateId(); n.title = title; n.message = message;
    n.level = level; n.timestampMs = QDateTime::currentMSecsSinceEpoch();
    n.durationMs = durationMs; n.dismissible = true; n.dismissed = false;
    m_active.prepend(n);
    m_history.prepend(n);
    while (m_history.size() > m_maxHistorySize) m_history.removeLast();
    ++m_totalShown;
    emit notificationShown(n);
    return n.id;
}
QString NotificationManager::showInfo(const QString &title, const QString &message, int durationMs) { return showNotification(title, message, NotificationLevel::Info, durationMs); }
QString NotificationManager::showWarning(const QString &title, const QString &message, int durationMs) { return showNotification(title, message, NotificationLevel::Warning, durationMs); }
QString NotificationManager::showError(const QString &title, const QString &message, int durationMs) { return showNotification(title, message, NotificationLevel::Error, durationMs); }
QString NotificationManager::showSuccess(const QString &title, const QString &message, int durationMs) { return showNotification(title, message, NotificationLevel::Success, durationMs); }

void NotificationManager::dismiss(const QString &id) {
    for (auto it = m_active.begin(); it != m_active.end(); ++it) {
        if (it->id == id) { it->dismissed = true; m_active.erase(it); ++m_totalDismissed; emit notificationDismissed(id); return; }
    }
}
void NotificationManager::dismissAll() {
    for (auto &n : m_active) n.dismissed = true;
    m_totalDismissed += m_active.size();
    m_active.clear();
}

QList<Notification> NotificationManager::activeNotifications() const { return m_active; }
QList<Notification> NotificationManager::recentHistory(int count) const { return m_history.mid(0, qMin(count, m_history.size())); }
int NotificationManager::activeCount() const { return m_active.size(); }
void NotificationManager::setMaxHistorySize(int maxSize) { m_maxHistorySize = qMax(1, maxSize); while (m_history.size() > m_maxHistorySize) m_history.removeLast(); }
int NotificationManager::maxHistorySize() const { return m_maxHistorySize; }

quint64 NotificationManager::totalShown() const { return m_totalShown; }
quint64 NotificationManager::totalDismissed() const { return m_totalDismissed; }
void NotificationManager::resetStatistics() { m_totalShown = 0; m_totalDismissed = 0; }

void NotificationManager::autoDismiss() {
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    auto it = m_active.begin();
    while (it != m_active.end()) {
        if (it->dismissible && (now - it->timestampMs) > it->durationMs) {
            QString id = it->id;
            it = m_active.erase(it);
            ++m_totalDismissed;
            emit notificationDismissed(id);
        } else { ++it; }
    }
}
QString NotificationManager::generateId() const { return QStringLiteral("notif_%1").arg(++m_idCounter); }
