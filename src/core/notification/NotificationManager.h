/**
 * @file NotificationManager.h
 * @brief 通知管理器 - 统一管理应用内通知、提醒和消息
 * @since score-132
 */
#ifndef NOTIFICATIONMANAGER_H
#define NOTIFICATIONMANAGER_H
#include <QList>
#include <QObject>
#include <QString>
#include <QTimer>

enum class NotificationLevel { Info, Warning, Error, Success };

struct Notification {
    QString id;
    QString title;
    QString message;
    NotificationLevel level = NotificationLevel::Info;
    qint64 timestampMs = 0;
    int durationMs = 5000;
    bool dismissible = true;
    bool dismissed = false;
};

class NotificationManager : public QObject {
    Q_OBJECT
public:
    static NotificationManager &instance();

    QString showNotification(const QString &title, const QString &message,
                             NotificationLevel level = NotificationLevel::Info,
                             int durationMs = 5000);
    QString showInfo(const QString &title, const QString &message, int durationMs = 3000);
    QString showWarning(const QString &title, const QString &message, int durationMs = 5000);
    QString showError(const QString &title, const QString &message, int durationMs = 8000);
    QString showSuccess(const QString &title, const QString &message, int durationMs = 3000);

    void dismiss(const QString &id);
    void dismissAll();

    QList<Notification> activeNotifications() const;
    QList<Notification> recentHistory(int count = 50) const;
    int activeCount() const;
    void setMaxHistorySize(int maxSize);
    int maxHistorySize() const;

    quint64 totalShown() const;
    quint64 totalDismissed() const;
    void resetStatistics();

signals:
    void notificationShown(const Notification &notification);
    void notificationDismissed(const QString &id);

private:
    explicit NotificationManager(QObject *parent = nullptr);
    ~NotificationManager() override;
    NotificationManager(const NotificationManager &) = delete;
    NotificationManager &operator=(const NotificationManager &) = delete;

    void autoDismiss();
    QString generateId() const;

    QList<Notification> m_active;
    QList<Notification> m_history;
    QTimer m_autoDismissTimer;
    int m_maxHistorySize = 200;

    mutable quint64 m_totalShown = 0;
    mutable quint64 m_totalDismissed = 0;
    mutable quint64 m_idCounter = 0;
};
#endif // NOTIFICATIONMANAGER_H
