#include "apps/serial_station/services/SerialProfileCatalogService.h"

#include <QtCore/QDir>
#include <QtCore/QVariant>

#include "utils/settings/SettingsManager.h"

namespace serial_station {

namespace {

constexpr int kMaxRecentProfiles = 8;
const char* kCatalogGroup = "serial_station/profiles";
const char* kRecentProfilesKey = "recent";
const char* kLastProfileKey = "last";

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
    m_settings->set(QStringLiteral("%1/%2").arg(QLatin1String(kCatalogGroup),
                                               QLatin1String(kLastProfileKey)),
                    normalizedPath);
    m_settings->sync();
    return true;
}

QStringList SerialProfileCatalogService::recentProfilePaths() const
{
    return normalizedRecentPaths();
}

QString SerialProfileCatalogService::lastProfilePath() const
{
    return normalizePath(m_settings->get(QStringLiteral("%1/%2").arg(QLatin1String(kCatalogGroup),
                                                                     QLatin1String(kLastProfileKey)))
                             .toString());
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
    if (rawPaths.isEmpty() && stored.typeId() == QMetaType::QString) {
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

} // namespace serial_station
