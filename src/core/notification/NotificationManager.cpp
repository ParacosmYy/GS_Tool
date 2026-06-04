/**
 * @file NotificationManager.cpp
 * @brief 通知管理器实现 — 活动通知队列+历史记录+未读计数
 */
#include "core/notification/NotificationManager.h"
#include <QDateTime>

/** @brief 构造函数 @param parent 父对象 */
NotificationManager::NotificationManager(QObject *parent) : QObject(parent) {}
/** @brief 析构函数 */
NotificationManager::~NotificationManager() = default;

/** @brief 发送通知 @param title 标题 @param msg 消息内容 @param prio 优先级 @return 通知ID */
QString NotificationManager::notify(const QString &title, const QString &msg, Priority prio) {
    QString id = QString("notif_%1").arg(++m_counter);
    Notification n;
    n.id = id; n.title = title; n.message = msg; n.priority = prio;
    n.timestamp = QDateTime::currentMSecsSinceEpoch(); n.acknowledged = false;
    m_active[id] = n;
    ++m_totalNotifications;
    if (prio == Critical) ++m_totalCriticalNotifications;
    if (prio == High) ++m_totalHighNotifications;
    if (prio == Normal) ++m_totalNormalNotifications;
    if (prio == Low) ++m_totalLowNotifications;
    m_history.prepend(n);
    pruneHistory();
    emit notificationAdded(n);
    emit unreadCountChanged(unreadCount());
    return id;
}

/** @brief 确认通知 @param id 通知ID */
void NotificationManager::acknowledge(const QString &id) {
    auto it = m_active.find(id);
    if (it != m_active.end()) { ++m_totalAcknowledges; it->acknowledged = true; emit notificationAcknowledged(id); emit unreadCountChanged(unreadCount()); }
}

/** @brief 关闭通知 @param id 通知ID */
void NotificationManager::dismiss(const QString &id) {
    ++m_totalDismisses; m_active.remove(id); emit notificationDismissed(id); emit unreadCountChanged(unreadCount());
}

/** @brief 清除所有通知和历史 */
void NotificationManager::clearAll() { ++m_totalClearAlls; m_active.clear(); m_history.clear(); emit unreadCountChanged(0); }

/** @brief 获取活动通知列表 @return 通知列表 */
QList<NotificationManager::Notification> NotificationManager::activeNotifications() const { return m_active.values(); }
/** @brief 获取所有历史通知 @return 通知列表 */
QList<NotificationManager::Notification> NotificationManager::allNotifications() const { return m_history; }

/** @brief 获取未读通知数量 @return 未读数量 */
int NotificationManager::unreadCount() const {
    int c = 0;
    for (auto it = m_active.constBegin(); it != m_active.constEnd(); ++it)
        if (!it->acknowledged) c++;
    return c;
}

/** @brief 设置历史记录最大容量 @param m 最大条目数 */
void NotificationManager::setMaxHistory(int m) { m_maxHistory = m; pruneHistory(); }

/** @brief 裁剪历史记录到最大容量 */
void NotificationManager::pruneHistory() {
    bool pruned = false;
    while (m_history.size() > m_maxHistory) { m_history.removeLast(); pruned = true; }
    if (pruned) ++m_totalHistoryPrunes;
}

/** @brief 重置通知统计计数器 */
void NotificationManager::resetNotificationStatistics() {
    m_totalNotifications = 0; m_totalAcknowledges = 0; m_totalDismisses = 0;
    m_totalClearAlls = 0; m_totalCriticalNotifications = 0;
    m_totalHighNotifications = 0; m_totalHistoryPrunes = 0;
    m_totalLowNotifications = 0; m_totalNormalNotifications = 0;
}
