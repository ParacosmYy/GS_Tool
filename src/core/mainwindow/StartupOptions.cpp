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

} // namespace

StartupOptions::StartupOptions(const QString& panelId, const QString& profileFilePath)
    : m_panelId(panelId.trimmed())
    , m_profileFilePath(profileFilePath.trimmed())
{
}

StartupOptions StartupOptions::fromArguments(const QStringList& arguments)
{
    QString panelId;
    QString profileFilePath;

    for (int index = 1; index < arguments.size(); ++index) {
        const QString argument = arguments.at(index).trimmed();
        if (argument == QStringLiteral("--panel")) {
            const QString value = valueAfterFlag(arguments, index);
            if (value.isEmpty()) {
                return StartupOptions({}, {});
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
                return StartupOptions({}, {});
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
                return StartupOptions({}, {});
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
    }
    if (!profileFilePath.isEmpty() && panelId.isEmpty()) {
        panelId = QStringLiteral("serial.station");
    }
    return StartupOptions(panelId, profileFilePath);
}

QString StartupOptions::panelId() const
{
    return m_panelId;
}

QString StartupOptions::profileFilePath() const
{
    return m_profileFilePath;
}
