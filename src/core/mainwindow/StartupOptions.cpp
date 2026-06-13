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

StartupOptions::StartupOptions(const QString& panelId)
    : m_panelId(panelId.trimmed())
{
}

StartupOptions StartupOptions::fromArguments(const QStringList& arguments)
{
    for (int index = 1; index < arguments.size(); ++index) {
        const QString argument = arguments.at(index).trimmed();
        if (argument == QStringLiteral("--panel")) {
            return StartupOptions(valueAfterFlag(arguments, index));
        }
        if (argument.startsWith(QStringLiteral("--panel="))) {
            return StartupOptions(argument.mid(QStringLiteral("--panel=").size()));
        }
        if (argument == QStringLiteral("--station")) {
            return StartupOptions(stationAliasToPanelId(valueAfterFlag(arguments, index)));
        }
        if (argument.startsWith(QStringLiteral("--station="))) {
            return StartupOptions(stationAliasToPanelId(argument.mid(QStringLiteral("--station=").size())));
        }
    }
    return StartupOptions();
}

QString StartupOptions::panelId() const
{
    return m_panelId;
}
