#pragma once
#include <QObject>
#include <QString>
#include <QVersionNumber>

class QNetworkAccessManager;
class QNetworkReply;

class UpdateManager : public QObject {
    Q_OBJECT
public:
    explicit UpdateManager(QObject *parent = nullptr);
    ~UpdateManager() override;

    void checkForUpdates();
    void setCurrentVersion(const QString &version);
    QString currentVersion() const;
    QString latestVersion() const;
    bool updateAvailable() const;
    QString downloadUrl() const;
    QString releaseNotes() const;

signals:
    void checkFinished(bool hasUpdate, const QString &latestVersion);
    void checkError(const QString &error);

private:
    void onCheckReply();
    QString m_currentVersion;
    QString m_latestVersion;
    QString m_downloadUrl;
    QString m_releaseNotes;
    QNetworkAccessManager *m_network = nullptr;
};
