#pragma once
#include <QObject>
#include <QMap>
#include <QString>
#include <QVariant>
#include <QDateTime>

class SessionManager : public QObject {
    Q_OBJECT
public:
    struct Session { QString id; QString name; QVariantMap state; qint64 created; qint64 modified; };
    explicit SessionManager(QObject *parent = nullptr);
    ~SessionManager() override;
    QString createSession(const QString &name);
    void deleteSession(const QString &id);
    void switchSession(const QString &id);
    void saveState(const QString &key, const QVariant &value);
    QVariant state(const QString &key) const;
    void saveAll(const QVariantMap &state);
    QVariantMap loadAll() const;
    Session currentSession() const;
    QList<Session> allSessions() const;
    QStringList sessionNames() const;
    void exportSession(const QString &id, const QString &path);
    QString importSession(const QString &path);
signals:
    void sessionCreated(const QString &id);
    void sessionDeleted(const QString &id);
    void sessionSwitched(const QString &id);
    void stateSaved(const QString &key);
private:
    QMap<QString, Session> m_sessions;
    QString m_currentId;
    int m_counter = 0;
};
