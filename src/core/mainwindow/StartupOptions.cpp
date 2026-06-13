/**
 * @file StartupOptions.cpp
 * @brief EmbedDebug 启动参数解析实现。
 */

#include "core/mainwindow/StartupOptions.h"

namespace {

QString valueAfterFlag(const QStringList& arguments, int index)
{
    const int valueIndex = index + 1;
    if (valueIndex >= arguments.size()) {
        return {};
    }

    const QString value = arguments.at(valueIndex).trimmed();
    if (value.startsWith(QStringLiteral("--"))) {
        return {};
    }
    return value;
}

QString stationAliasToPanelId(const QString& station)
{
    const QString normalized = station.trimmed().toLower();
    if (normalized == QStringLiteral("serial")
        || normalized == QStringLiteral("serial_station")
        || normalized == QStringLiteral("serial-station")) {
        return QStringLiteral("serial.station");
    }
    return {};
}

bool isLastProfileFlagArgument(const QString& argument)
{
    return argument == QStringLiteral("--last-profile")
        || argument == QStringLiteral("--serial-last-profile")
        || argument.startsWith(QStringLiteral("--last-profile="))
        || argument.startsWith(QStringLiteral("--serial-last-profile="));
}

bool parseBooleanFlagValue(const QString& rawValue, bool* ok)
{
    const QString normalized = rawValue.trimmed().toLower();
    if (normalized == QStringLiteral("1")
        || normalized == QStringLiteral("true")
        || normalized == QStringLiteral("yes")
        || normalized == QStringLiteral("on")) {
        *ok = true;
        return true;
    }
    if (normalized == QStringLiteral("0")
        || normalized == QStringLiteral("false")
        || normalized == QStringLiteral("no")
        || normalized == QStringLiteral("off")
        || normalized.isEmpty()) {
        *ok = true;
        return false;
    }

    *ok = false;
    return false;
}

} // namespace

StartupOptions::StartupOptions(const QString& panelId,
                               const QString& profileFilePath,
                               bool loadLastProfile)
    : m_panelId(panelId.trimmed())
    , m_profileFilePath(profileFilePath.trimmed())
    , m_loadLastProfile(loadLastProfile)
{
}

StartupOptions StartupOptions::fromArguments(const QStringList& arguments)
{
    QString panelId;
    QString profileFilePath;
    bool loadLastProfile = false;

    for (int index = 1; index < arguments.size(); ++index) {
        const QString argument = arguments.at(index).trimmed();
        if (argument == QStringLiteral("--panel")) {
            const QString value = valueAfterFlag(arguments, index);
            if (value.isEmpty()) {
                return StartupOptions({}, {}, false);
            }
            panelId = value;
            ++index;
            continue;
        }
        if (argument.startsWith(QStringLiteral("--panel="))) {
            panelId = argument.mid(QStringLiteral("--panel=").size()).trimmed();
            continue;
        }
        if (argument == QStringLiteral("--station")) {
            const QString value = valueAfterFlag(arguments, index);
            if (value.isEmpty()) {
                return StartupOptions({}, {}, false);
            }
            panelId = stationAliasToPanelId(value);
            ++index;
            continue;
        }
        if (argument.startsWith(QStringLiteral("--station="))) {
            panelId = stationAliasToPanelId(argument.mid(QStringLiteral("--station=").size()));
            continue;
        }
        if (argument == QStringLiteral("--profile") || argument == QStringLiteral("--serial-profile")) {
            const QString value = valueAfterFlag(arguments, index);
            if (value.isEmpty()) {
                const int nextIndex = index + 1;
                if (nextIndex < arguments.size()
                    && isLastProfileFlagArgument(arguments.at(nextIndex).trimmed())) {
                    continue;
                }
                return StartupOptions({}, {}, false);
            }
            profileFilePath = value;
            ++index;
            continue;
        }
        if (argument.startsWith(QStringLiteral("--profile="))) {
            profileFilePath = argument.mid(QStringLiteral("--profile=").size()).trimmed();
            continue;
        }
        if (argument.startsWith(QStringLiteral("--serial-profile="))) {
            profileFilePath = argument.mid(QStringLiteral("--serial-profile=").size()).trimmed();
            continue;
        }
        if (argument == QStringLiteral("--last-profile")
            || argument == QStringLiteral("--serial-last-profile")) {
            loadLastProfile = true;
            continue;
        }
        if (argument.startsWith(QStringLiteral("--last-profile="))) {
            bool ok = false;
            const bool value = parseBooleanFlagValue(argument.mid(QStringLiteral("--last-profile=").size()), &ok);
            loadLastProfile = ok ? value : false;
            continue;
        }
        if (argument.startsWith(QStringLiteral("--serial-last-profile="))) {
            bool ok = false;
            const bool value = parseBooleanFlagValue(argument.mid(QStringLiteral("--serial-last-profile=").size()), &ok);
            loadLastProfile = ok ? value : false;
            continue;
        }
    }
    if ((!profileFilePath.isEmpty() || loadLastProfile) && panelId.isEmpty()) {
        panelId = QStringLiteral("serial.station");
    }
    return StartupOptions(panelId, profileFilePath, loadLastProfile);
}

QString StartupOptions::panelId() const
{
    return m_panelId;
}

QString StartupOptions::profileFilePath() const
{
    return m_profileFilePath;
}

bool StartupOptions::loadLastProfile() const
{
    return m_loadLastProfile;
}
