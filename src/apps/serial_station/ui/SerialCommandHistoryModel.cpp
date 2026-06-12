#include "apps/serial_station/ui/SerialCommandHistoryModel.h"

#include <QtCore/QtGlobal>

namespace serial_station {

SerialCommandHistoryModel::SerialCommandHistoryModel(int maxItems)
{
    setMaxItems(maxItems);
}

bool SerialCommandHistoryModel::recordCommand(const QString& command, const QString& mode)
{
    const QString normalized = normalizedCommand(command);
    if (normalized.isEmpty()) {
        return false;
    }

    const QString normalizedModeValue = normalizedMode(mode);
    const int existingIndex = indexOf(normalized, normalizedModeValue);
    SerialCommandHistoryEntry entry;
    if (existingIndex >= 0) {
        entry = m_entries.takeAt(existingIndex);
        ++entry.useCount;
    } else {
        entry.command = normalized;
        entry.mode = normalizedModeValue;
        entry.useCount = 1;
    }

    entry.lastUsed = QDateTime::currentDateTimeUtc();
    m_entries.prepend(entry);
    trimToLimit();
    return true;
}

void SerialCommandHistoryModel::clear()
{
    m_entries.clear();
}

void SerialCommandHistoryModel::setMaxItems(int maxItems)
{
    m_maxItems = qMax(1, maxItems);
    trimToLimit();
}

int SerialCommandHistoryModel::maxItems() const
{
    return m_maxItems;
}

int SerialCommandHistoryModel::count() const
{
    return m_entries.count();
}

bool SerialCommandHistoryModel::isEmpty() const
{
    return m_entries.isEmpty();
}

QVector<SerialCommandHistoryEntry> SerialCommandHistoryModel::entries() const
{
    return m_entries;
}

QStringList SerialCommandHistoryModel::commands() const
{
    QStringList result;
    result.reserve(m_entries.count());
    for (const SerialCommandHistoryEntry& entry : m_entries) {
        result.append(entry.command);
    }
    return result;
}

QString SerialCommandHistoryModel::commandAt(int index) const
{
    if (index < 0 || index >= m_entries.count()) {
        return {};
    }
    return m_entries.at(index).command;
}

QString SerialCommandHistoryModel::modeAt(int index) const
{
    if (index < 0 || index >= m_entries.count()) {
        return {};
    }
    return m_entries.at(index).mode;
}

QString SerialCommandHistoryModel::displayTextAt(int index) const
{
    if (index < 0 || index >= m_entries.count()) {
        return {};
    }

    const SerialCommandHistoryEntry& entry = m_entries.at(index);
    return QStringLiteral("%1 | %2 | %3x")
        .arg(displayMode(entry.mode), entry.command, QString::number(entry.useCount));
}

int SerialCommandHistoryModel::indexOf(const QString& command, const QString& mode) const
{
    const QString normalized = normalizedCommand(command);
    const QString normalizedModeValue = normalizedMode(mode);
    for (int i = 0; i < m_entries.count(); ++i) {
        const SerialCommandHistoryEntry& entry = m_entries.at(i);
        if (entry.command == normalized && entry.mode == normalizedModeValue) {
            return i;
        }
    }
    return -1;
}

bool SerialCommandHistoryModel::contains(const QString& command, const QString& mode) const
{
    return indexOf(command, mode) >= 0;
}

QString SerialCommandHistoryModel::normalizedCommand(const QString& command)
{
    return command.trimmed();
}

QString SerialCommandHistoryModel::normalizedMode(const QString& mode)
{
    const QString normalized = mode.trimmed().toLower();
    if (normalized == QStringLiteral("hex") || normalized == QStringLiteral("protocol")) {
        return normalized;
    }
    return QStringLiteral("ascii");
}

QString SerialCommandHistoryModel::displayMode(const QString& mode)
{
    const QString normalized = normalizedMode(mode);
    if (normalized == QStringLiteral("hex")) {
        return QStringLiteral("HEX");
    }
    if (normalized == QStringLiteral("protocol")) {
        return QStringLiteral("协议");
    }
    return QStringLiteral("ASCII");
}

void SerialCommandHistoryModel::trimToLimit()
{
    while (m_entries.count() > m_maxItems) {
        m_entries.removeLast();
    }
}

} // namespace serial_station
