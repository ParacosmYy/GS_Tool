#include "apps/serial_station/services/SerialProfileCatalogService.h"

#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QVariant>

#include "utils/settings/SettingsManager.h"

namespace serial_station {

namespace {

constexpr int kMaxRecentProfiles = 8;
const char* kCatalogGroup = "serial_station/profiles";
const char* kRecentProfilesKey = "recent";
const char* kLastProfileKey = "last";
const char* kDefaultProfileDirectoryKey = "defaultDirectory";

} // namespace

SerialProfileCatalogService::SerialProfileCatalogService(SettingsManager* settings)
    : m_settings(settings ? settings : &SettingsManager::instance())
{
}

bool SerialProfileCatalogService::recordProfilePath(const QString& filePath)
{
    const QString normalizedPath = normalizePath(filePath);
    if (normalizedPath.isEmpty()) {
        return false;
    }

    QStringList paths = normalizedRecentPaths();
    paths.removeAll(normalizedPath);
    paths.prepend(normalizedPath);
    while (paths.size() > kMaxRecentProfiles) {
        paths.removeLast();
    }

    writeRecentPaths(paths);
    writeLastProfilePath(normalizedPath);
    m_settings->sync();
    return true;
}

QStringList SerialProfileCatalogService::recentProfilePaths() const
{
    return normalizedRecentPaths();
}

bool SerialProfileCatalogService::containsProfilePath(const QString& filePath) const
{
    const QString normalizedPath = normalizePath(filePath);
    if (normalizedPath.isEmpty()) {
        return false;
    }

    return normalizedRecentPaths().contains(normalizedPath);
}

bool SerialProfileCatalogService::removeProfilePath(const QString& filePath)
{
    const QString normalizedPath = normalizePath(filePath);
    if (normalizedPath.isEmpty()) {
        return false;
    }

    QStringList paths = normalizedRecentPaths();
    if (!paths.removeOne(normalizedPath)) {
        return false;
    }

    writeRecentPaths(paths);
    const QString currentLastPath = lastProfilePath();
    if (currentLastPath == normalizedPath || !paths.contains(currentLastPath)) {
        writeLastProfilePath(paths.isEmpty() ? QString() : paths.first());
    }

    m_settings->sync();
    return true;
}

int SerialProfileCatalogService::pruneMissingProfilePaths()
{
    const QStringList paths = normalizedRecentPaths();
    QStringList existingPaths;
    int removedCount = 0;
    for (const QString& path : paths) {
        if (QFileInfo::exists(path)) {
            existingPaths.append(path);
        } else {
            ++removedCount;
        }
    }

    if (removedCount > 0) {
        writeRecentPaths(existingPaths);
    }

    const QString currentLastPath = lastProfilePath();
    const bool hasUsableLast = !currentLastPath.isEmpty()
        && existingPaths.contains(currentLastPath)
        && QFileInfo::exists(currentLastPath);
    if (!hasUsableLast) {
        writeLastProfilePath(existingPaths.isEmpty() ? QString() : existingPaths.first());
    }

    if (removedCount > 0 || !hasUsableLast) {
        m_settings->sync();
    }
    return removedCount;
}

QString SerialProfileCatalogService::lastProfilePath() const
{
    return normalizePath(m_settings->get(QStringLiteral("%1/%2").arg(QLatin1String(kCatalogGroup),
                                                                     QLatin1String(kLastProfileKey)))
                             .toString());
}

QStringList SerialProfileCatalogService::discoverProfilePaths(const QString& directoryPath) const
{
    const QString normalizedDirectory = normalizePath(directoryPath);
    if (normalizedDirectory.isEmpty()) {
        return {};
    }

    QDir directory(normalizedDirectory);
    if (!directory.exists()) {
        return {};
    }

    const QFileInfoList entries = directory.entryInfoList(
        {QStringLiteral("*.edserialprofile"), QStringLiteral("*.json")},
        QDir::Files | QDir::Readable,
        QDir::Name | QDir::IgnoreCase);

    QStringList paths;
    for (const QFileInfo& entry : entries) {
        const QString path = normalizePath(entry.absoluteFilePath());
        if (!path.isEmpty() && !paths.contains(path)) {
            paths.append(path);
        }
    }
    return paths;
}

int SerialProfileCatalogService::importProfileDirectory(const QString& directoryPath)
{
    int importedCount = 0;
    const QStringList paths = discoverProfilePaths(directoryPath);
    for (const QString& path : paths) {
        const bool alreadyKnown = containsProfilePath(path);
        if (recordProfilePath(path) && !alreadyKnown) {
            ++importedCount;
        }
    }
    return importedCount;
}

bool SerialProfileCatalogService::setDefaultProfileDirectory(const QString& directoryPath)
{
    const QString normalizedPath = normalizePath(directoryPath);
    if (normalizedPath.isEmpty()) {
        return false;
    }

    m_settings->set(QStringLiteral("%1/%2").arg(QLatin1String(kCatalogGroup),
                                                QLatin1String(kDefaultProfileDirectoryKey)),
                    normalizedPath);
    m_settings->sync();
    return true;
}

QString SerialProfileCatalogService::defaultProfileDirectory() const
{
    return normalizePath(m_settings->get(QStringLiteral("%1/%2").arg(
                                             QLatin1String(kCatalogGroup),
                                             QLatin1String(kDefaultProfileDirectoryKey)))
                             .toString());
}

void SerialProfileCatalogService::clearDefaultProfileDirectory()
{
    m_settings->remove(QStringLiteral("%1/%2").arg(QLatin1String(kCatalogGroup),
                                                   QLatin1String(kDefaultProfileDirectoryKey)));
    m_settings->sync();
}

void SerialProfileCatalogService::clear()
{
    m_settings->removeGroup(QString::fromLatin1(kCatalogGroup));
    m_settings->sync();
}

QString SerialProfileCatalogService::normalizePath(const QString& filePath) const
{
    const QString trimmed = filePath.trimmed();
    if (trimmed.isEmpty()) {
        return {};
    }

    return QDir::cleanPath(trimmed);
}

QStringList SerialProfileCatalogService::normalizedRecentPaths() const
{
    const QVariant stored = m_settings->get(QStringLiteral("%1/%2").arg(QLatin1String(kCatalogGroup),
                                                                        QLatin1String(kRecentProfilesKey)));
    QStringList rawPaths = stored.toStringList();
    if ((rawPaths.isEmpty() && stored.typeId() == QMetaType::QString)
        || (rawPaths.size() == 1 && rawPaths.first().contains(QLatin1Char('|')))) {
        rawPaths = stored.toString().split(QLatin1Char('|'), Qt::SkipEmptyParts);
    }

    QStringList paths;
    for (const QString& rawPath : rawPaths) {
        const QString normalizedPath = normalizePath(rawPath);
        if (!normalizedPath.isEmpty() && !paths.contains(normalizedPath)) {
            paths.append(normalizedPath);
        }
    }
    while (paths.size() > kMaxRecentProfiles) {
        paths.removeLast();
    }
    return paths;
}

void SerialProfileCatalogService::writeRecentPaths(const QStringList& paths)
{
    m_settings->set(QStringLiteral("%1/%2").arg(QLatin1String(kCatalogGroup),
                                                QLatin1String(kRecentProfilesKey)),
                    paths);
}

void SerialProfileCatalogService::writeLastProfilePath(const QString& path)
{
    const QString key = QStringLiteral("%1/%2").arg(QLatin1String(kCatalogGroup),
                                                    QLatin1String(kLastProfileKey));
    if (path.isEmpty()) {
        m_settings->remove(key);
    } else {
        m_settings->set(key, path);
    }
}

} // namespace serial_station
