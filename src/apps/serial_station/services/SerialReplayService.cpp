#include "apps/serial_station/services/SerialReplayService.h"

#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QRegularExpression>
#include <QtCore/QStringList>

#include <cmath>

namespace serial_station {

namespace {

QString directionLabel(SerialLogDirection direction)
{
    return SerialLogService::directionName(direction).toUpper();
}

QString timestampText(const QDateTime& timestamp)
{
    if (!timestamp.isValid()) {
        return QStringLiteral("-");
    }

    return timestamp.toString(Qt::ISODateWithMs);
}

double normalizedSpeed(double speedMultiplier)
{
    if (!std::isfinite(speedMultiplier) || speedMultiplier <= 0.0) {
        return 1.0;
    }

    return speedMultiplier;
}

QString compactHexText(const QString& text)
{
    QString compact = text;
    compact.remove(QRegularExpression(QStringLiteral("\\s+")));
    return compact;
}

} // namespace

SerialReplayPlan SerialReplayService::buildPlan(const QVector<SerialLogRecord>& records,
                                                const SerialReplayOptions& options) const
{
    if (records.isEmpty()) {
        return makeFailure(QStringLiteral("没有可回放的日志记录"));
    }

    return makePlan(records, options);
}

SerialReplayPlan SerialReplayService::buildPlanFromJsonLines(
    const QString& jsonLines,
    const SerialReplayOptions& options) const
{
    const QStringList lines = jsonLines.split(QLatin1Char('\n'));
    QVector<SerialLogRecord> records;
    records.reserve(lines.count());

    for (int index = 0; index < lines.count(); ++index) {
        const QString line = lines.at(index).trimmed();
        if (line.isEmpty()) {
            continue;
        }

        SerialLogRecord record;
        QString errorMessage;
        if (!parseJsonLine(line, index + 1, record, errorMessage)) {
            return makeFailure(errorMessage);
        }

        records.append(record);
    }

    if (records.isEmpty()) {
        return makeFailure(QStringLiteral("JSON Lines 中没有可回放记录"));
    }

    return makePlan(records, options);
}

QString SerialReplayService::eventSummary(const SerialReplayEvent& event) const
{
    QStringList parts;
    parts << QStringLiteral("+%1ms").arg(event.delayMs);
    parts << directionLabel(event.direction);
    parts << timestampText(event.timestamp);
    parts << event.source;

    if (!event.text.isEmpty()) {
        parts << event.text;
    }

    const QString payloadHex = SerialLogService::payloadHex(event.payload);
    if (!payloadHex.isEmpty()) {
        parts << QStringLiteral("hex=%1").arg(payloadHex);
    }

    return parts.join(QLatin1Char(' '));
}

QString SerialReplayService::planSummary(const SerialReplayPlan& plan) const
{
    if (!plan.ok) {
        return QStringLiteral("Replay failed: %1").arg(plan.errorMessage);
    }

    return QStringLiteral("Replay plan: %1 events, %2 ms, skipped %3")
        .arg(QString::number(plan.events.count()),
             QString::number(plan.totalDurationMs),
             QString::number(plan.skippedRecords));
}

bool SerialReplayService::acceptsDirection(SerialLogDirection direction,
                                           const SerialReplayOptions& options) const
{
    if (direction == SerialLogDirection::Tx) {
        return options.includeTx;
    }

    if (direction == SerialLogDirection::Rx) {
        return options.includeRx;
    }

    if (direction == SerialLogDirection::Error) {
        return options.includeError;
    }

    return options.includeSystem;
}

qint64 SerialReplayService::scaledDelayMs(qint64 rawDelayMs,
                                          const SerialReplayOptions& options) const
{
    const qint64 clampedRaw = qMax<qint64>(0, rawDelayMs);
    const double speed = normalizedSpeed(options.speedMultiplier);
    qint64 delay = static_cast<qint64>(std::llround(static_cast<double>(clampedRaw) / speed));

    if (options.maxDelayMs >= 0) {
        delay = qMin(delay, options.maxDelayMs);
    }

    return qMax<qint64>(0, delay);
}

SerialReplayPlan SerialReplayService::makeFailure(const QString& message) const
{
    SerialReplayPlan plan;
    plan.errorMessage = message;
    return plan;
}

SerialReplayPlan SerialReplayService::makePlan(const QVector<SerialLogRecord>& records,
                                               const SerialReplayOptions& options) const
{
    SerialReplayPlan plan;
    QDateTime previousTimestamp;

    for (const SerialLogRecord& record : records) {
        if (!acceptsDirection(record.direction, options)) {
            ++plan.skippedRecords;
            continue;
        }

        SerialReplayEvent event;
        event.direction = record.direction;
        event.timestamp = record.timestamp;
        event.source = record.source;
        event.text = record.text;
        event.payload = record.payload;

        if (!previousTimestamp.isValid() || !event.timestamp.isValid()) {
            event.delayMs = 0;
        } else {
            event.delayMs = scaledDelayMs(previousTimestamp.msecsTo(event.timestamp), options);
        }

        previousTimestamp = event.timestamp;
        plan.totalDurationMs += event.delayMs;
        plan.events.append(event);
    }

    if (plan.events.isEmpty()) {
        return makeFailure(QStringLiteral("过滤后没有可回放事件"));
    }

    plan.ok = true;
    return plan;
}

bool SerialReplayService::parseJsonLine(const QString& line,
                                        int lineNumber,
                                        SerialLogRecord& record,
                                        QString& errorMessage) const
{
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(line.toUtf8(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        errorMessage = QStringLiteral("第 %1 行 JSON 无效: %2")
                           .arg(QString::number(lineNumber), parseError.errorString());
        return false;
    }

    const QJsonObject object = document.object();
    if (!parseDirection(object.value(QStringLiteral("direction")).toString(),
                        record.direction,
                        errorMessage)) {
        errorMessage = QStringLiteral("第 %1 行 %2").arg(QString::number(lineNumber), errorMessage);
        return false;
    }

    record.text = object.value(QStringLiteral("text")).toString().trimmed();
    if (record.text.isEmpty()) {
        errorMessage = QStringLiteral("第 %1 行缺少 text").arg(QString::number(lineNumber));
        return false;
    }

    record.source = object.value(QStringLiteral("source")).toString().trimmed();
    if (record.source.isEmpty()) {
        record.source = QStringLiteral("serial_station");
    }

    const QString timestamp = object.value(QStringLiteral("timestamp")).toString();
    record.timestamp = QDateTime::fromString(timestamp, Qt::ISODateWithMs);

    if (!parsePayloadHex(object.value(QStringLiteral("payloadHex")).toString(),
                         record.payload,
                         errorMessage)) {
        errorMessage = QStringLiteral("第 %1 行 %2").arg(QString::number(lineNumber), errorMessage);
        return false;
    }

    return true;
}

bool SerialReplayService::parseDirection(const QString& text,
                                         SerialLogDirection& direction,
                                         QString& errorMessage) const
{
    const QString normalized = text.trimmed().toLower();
    if (normalized == QStringLiteral("tx")) {
        direction = SerialLogDirection::Tx;
        return true;
    }

    if (normalized == QStringLiteral("rx")) {
        direction = SerialLogDirection::Rx;
        return true;
    }

    if (normalized == QStringLiteral("system")) {
        direction = SerialLogDirection::System;
        return true;
    }

    if (normalized == QStringLiteral("error")) {
        direction = SerialLogDirection::Error;
        return true;
    }

    errorMessage = QStringLiteral("direction 无效");
    return false;
}

bool SerialReplayService::parsePayloadHex(const QString& text,
                                          QByteArray& payload,
                                          QString& errorMessage) const
{
    const QString compact = compactHexText(text);
    if (compact.isEmpty()) {
        payload.clear();
        return true;
    }

    if ((compact.size() % 2) != 0) {
        errorMessage = QStringLiteral("payloadHex 长度不是偶数");
        return false;
    }

    const QRegularExpression hexPattern(QStringLiteral("^[0-9a-fA-F]+$"));
    if (!hexPattern.match(compact).hasMatch()) {
        errorMessage = QStringLiteral("payloadHex 包含非法字符");
        return false;
    }

    payload = QByteArray::fromHex(compact.toLatin1());
    return true;
}

} // namespace serial_station
