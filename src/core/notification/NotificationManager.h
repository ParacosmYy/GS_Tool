#pragma once
#include <QObject>
#include <QMap>
#include <QString>
#include <QList>
#include <QTimer>

class NotificationManager : public QObject {
    Q_OBJECT
public:
    enum Priority { Low, Normal, High, Critical };
    Q_ENUM(Priority)
    struct Notification {
        QString id;
        QString title;
        QString message;
        Priority priority;
        qint64 timestamp;
        bool acknowledged;
    };

    explicit NotificationManager(QObject *parent = nullptr);
    ~NotificationManager() override;
    QString notify(const QString &title, const QString &msg, Priority prio = Normal);
    void acknowledge(const QString &id);
    void dismiss(const QString &id);
    void clearAll();
    QList<Notification> activeNotifications() const;
    QList<Notification> allNotifications() const;
    int unreadCount() const;
    void setMaxHistory(int max);
signals:
    void notificationAdded(const Notification &n);
    void notificationAcknowledged(const QString &id);
    void notificationDismissed(const QString &id);
    void unreadCountChanged(int count);
private:
    void pruneHistory();
    QMap<QString, Notification> m_active;
    QList<Notification> m_history;
    int m_maxHistory = 100;
    int m_counter = 0;
};
