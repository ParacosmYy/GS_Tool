#include "apps/serial_station/services/SerialLogService.h"

#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QStringList>

namespace serial_station {

namespace {

QString normalizedSource(const QString& source)
{
    const QString trimmed = source.trimmed();
    if (trimmed.isEmpty()) {
        return QStringLiteral("serial_station");
    }

    return trimmed;
}

QString timestampText(const QDateTime& timestamp)
{
    return timestamp.toString(Qt::ISODateWithMs);
}

QString joinedLines(const QStringList& lines)
{
    if (lines.isEmpty()) {
        return QString();
    }

    return lines.join(QLatin1Char('\n'));
}

} // namespace

SerialLogService::SerialLogService(int maxRecords)
{
    setMaxRecords(maxRecords);
}

bool SerialLogService::append(const SerialLogRecord& record)
{
    const SerialLogRecord normalized = normalizedRecord(record);
    if (!isMeaningful(normalized)) {
        return false;
    }

    m_records.append(normalized);
    trimOverflow();
    return true;
}

bool SerialLogService::appendTx(const QString& text,
                                const QByteArray& payload,
                                const QString& source,
                                const QVariantMap& fields)
{
    SerialLogRecord record;
    record.direction = SerialLogDirection::Tx;
    record.source = source;
    record.text = text;
    record.payload = payload;
    record.fields = fields;
    return append(record);
}

bool SerialLogService::appendRx(const QString& text,
                                const QByteArray& payload,
                                const QString& source,
                                const QVariantMap& fields)
{
    SerialLogRecord record;
    record.direction = SerialLogDirection::Rx;
    record.source = source;
    record.text = text;
    record.payload = payload;
    record.fields = fields;
    return append(record);
}

bool SerialLogService::appendSystem(const QString& text,
                                    const QString& source,
                                    const QVariantMap& fields)
{
    SerialLogRecord record;
    record.direction = SerialLogDirection::System;
    record.source = source;
    record.text = text;
    record.fields = fields;
    return append(record);
}

bool SerialLogService::appendError(const QString& text,
                                   const QString& source,
                                   const QVariantMap& fields)
{
    SerialLogRecord record;
    record.direction = SerialLogDirection::Error;
    record.source = source;
    record.text = text;
    record.fields = fields;
    return append(record);
}

void SerialLogService::clear()
{
    m_records.clear();
}

int SerialLogService::count() const
{
    return m_records.count();
}

bool SerialLogService::isEmpty() const
{
    return m_records.isEmpty();
}

int SerialLogService::maxRecords() const
{
    return m_maxRecords;
}

void SerialLogService::setMaxRecords(int maxRecords)
{
    m_maxRecords = qMax(1, maxRecords);
    trimOverflow();
}

QVector<SerialLogRecord> SerialLogService::records() const
{
    return m_records;
}

QVector<SerialLogRecord> SerialLogService::records(const SerialLogFilter& filter) const
{
    QVector<SerialLogRecord> filtered;
    filtered.reserve(m_records.count());

    for (const SerialLogRecord& record : m_records) {
        if (matchesFilter(record, filter)) {
            filtered.append(record);
        }
    }

    return filtered;
}

QString SerialLogService::toPlainText() const
{
    SerialLogFilter filter;
    return toPlainText(filter);
}

QString SerialLogService::toPlainText(const SerialLogFilter& filter) const
{
    QStringList lines;
    const QVector<SerialLogRecord> filteredRecords = records(filter);
    lines.reserve(filteredRecords.count());

    for (const SerialLogRecord& record : filteredRecords) {
        lines.append(recordToPlainTextLine(record));
    }

    return joinedLines(lines);
}

QString SerialLogService::toJsonLines() const
{
    SerialLogFilter filter;
    return toJsonLines(filter);
}

QString SerialLogService::toJsonLines(const SerialLogFilter& filter) const
{
    QStringList lines;
    const QVector<SerialLogRecord> filteredRecords = records(filter);
    lines.reserve(filteredRecords.count());

    for (const SerialLogRecord& record : filteredRecords) {
        lines.append(recordToJsonLine(record));
    }

    return joinedLines(lines);
}

QString SerialLogService::directionName(SerialLogDirection direction)
{
    switch (direction) {
    case SerialLogDirection::Rx:
        return QStringLiteral("rx");
    case SerialLogDirection::Tx:
        return QStringLiteral("tx");
    case SerialLogDirection::System:
        return QStringLiteral("system");
    case SerialLogDirection::Error:
        return QStringLiteral("error");
    }

    return QStringLiteral("system");
}

QString SerialLogService::payloadHex(const QByteArray& payload)
{
    if (payload.isEmpty()) {
        return QString();
    }

    return QString::fromLatin1(payload.toHex(' ').toUpper());
}

SerialLogRecord SerialLogService::normalizedRecord(const SerialLogRecord& record)
{
    SerialLogRecord normalized = record;
    if (!normalized.timestamp.isValid()) {
        normalized.timestamp = QDateTime::currentDateTime();
    }

    normalized.source = normalizedSource(normalized.source);
    normalized.text = normalized.text.trimmed();
    return normalized;
}

bool SerialLogService::isMeaningful(const SerialLogRecord& record)
{
    return !record.text.isEmpty() || !record.payload.isEmpty() || !record.fields.isEmpty();
}

bool SerialLogService::matchesFilter(const SerialLogRecord& record,
                                     const SerialLogFilter& filter)
{
    if (!directionAccepted(record.direction, filter.directions)) {
        return false;
    }

    if (!filter.sourceContains.trimmed().isEmpty() &&
        !record.source.contains(filter.sourceContains.trimmed(), Qt::CaseInsensitive)) {
        return false;
    }

    if (!filter.textContains.trimmed().isEmpty() &&
        !record.text.contains(filter.textContains.trimmed(), Qt::CaseInsensitive) &&
        !payloadHex(record.payload).contains(filter.textContains.trimmed(), Qt::CaseInsensitive)) {
        return false;
    }

    if (filter.from.isValid() && record.timestamp < filter.from) {
        return false;
    }

    if (filter.to.isValid() && record.timestamp > filter.to) {
        return false;
    }

    return true;
}

bool SerialLogService::directionAccepted(SerialLogDirection direction,
                                         const QVector<SerialLogDirection>& directions)
{
    if (directions.isEmpty()) {
        return true;
    }

    for (const SerialLogDirection accepted : directions) {
        if (accepted == direction) {
            return true;
        }
    }

    return false;
}

QString SerialLogService::recordToPlainTextLine(const SerialLogRecord& record)
{
    QStringList parts;
    parts << QStringLiteral("[%1]").arg(timestampText(record.timestamp));
    parts << directionName(record.direction).toUpper().leftJustified(6);
    parts << record.source;

    if (!record.text.isEmpty()) {
        parts << record.text;
    }

    const QString hex = payloadHex(record.payload);
    if (!hex.isEmpty()) {
        parts << QStringLiteral("hex=%1").arg(hex);
    }

    return parts.join(QLatin1Char(' '));
}

QString SerialLogService::recordToJsonLine(const SerialLogRecord& record)
{
    QJsonObject object;
    object.insert(QStringLiteral("timestamp"), timestampText(record.timestamp));
    object.insert(QStringLiteral("direction"), directionName(record.direction));
    object.insert(QStringLiteral("source"), record.source);
    object.insert(QStringLiteral("text"), record.text);
    object.insert(QStringLiteral("payloadHex"), payloadHex(record.payload));
    object.insert(QStringLiteral("fields"), QJsonObject::fromVariantMap(record.fields));

    const QJsonDocument document(object);
    return QString::fromUtf8(document.toJson(QJsonDocument::Compact));
}

void SerialLogService::trimOverflow()
{
    const int overflow = m_records.count() - m_maxRecords;
    if (overflow <= 0) {
        return;
    }

    m_records.erase(m_records.begin(), m_records.begin() + overflow);
}

} // namespace serial_station
