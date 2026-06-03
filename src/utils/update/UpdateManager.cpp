#include "utils/update/UpdateManager.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>

UpdateManager::UpdateManager(QObject *parent)
    : QObject(parent), m_network(new QNetworkAccessManager(this)) {}
UpdateManager::~UpdateManager() = default;

void UpdateManager::checkForUpdates() {
    QUrl url("https://api.github.com/repos/embeddebug/embeddebug/releases/latest");
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::UserAgentHeader, "EmbedDebug");
    auto *reply = m_network->get(req);
    connect(reply, &QNetworkReply::finished, this, &UpdateManager::onCheckReply);
}

void UpdateManager::onCheckReply() {
    auto *reply = qobject_cast<QNetworkReply *>(sender());
    if (!reply) return;
    reply->deleteLater();
    if (reply->error() != QNetworkReply::NoError) {
        emit checkError(reply->errorString());
        return;
    }
    auto doc = QJsonDocument::fromJson(reply->readAll());
    if (!doc.isObject()) { emit checkError(tr("Invalid response")); return; }
    auto obj = doc.object();
    m_latestVersion = obj["tag_name"].toString();
    m_releaseNotes = obj["body"].toString();
    auto assets = obj["assets"].toArray();
    if (!assets.isEmpty())
        m_downloadUrl = assets[0].toObject()["browser_download_url"].toString();
    bool has = QVersionNumber::fromString(m_latestVersion)
               > QVersionNumber::fromString(m_currentVersion);
    emit checkFinished(has, m_latestVersion);
}

void UpdateManager::setCurrentVersion(const QString &v) { m_currentVersion = v; }
QString UpdateManager::currentVersion() const { return m_currentVersion; }
QString UpdateManager::latestVersion() const { return m_latestVersion; }
bool UpdateManager::updateAvailable() const {
    return QVersionNumber::fromString(m_latestVersion)
           > QVersionNumber::fromString(m_currentVersion);
}
QString UpdateManager::downloadUrl() const { return m_downloadUrl; }
QString UpdateManager::releaseNotes() const { return m_releaseNotes; }
