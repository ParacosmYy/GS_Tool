#ifndef SERIAL_REPLAY_SERVICE_H
#define SERIAL_REPLAY_SERVICE_H

#include <QtCore/QByteArray>
#include <QtCore/QDateTime>
#include <QtCore/QString>
#include <QtCore/QVector>

#include "apps/serial_station/services/SerialLogService.h"

namespace serial_station {

/**
 * @brief Serial Station 日志回放选项。
 */
struct SerialReplayOptions {
    bool includeTx = true;
    bool includeRx = true;
    bool includeSystem = false;
    bool includeError = false;
    double speedMultiplier = 1.0;
    qint64 maxDelayMs = 5000;
};

/**
 * @brief Serial Station 单条回放事件。
 */
struct SerialReplayEvent {
    SerialLogDirection direction = SerialLogDirection::System;
    QDateTime timestamp;
    QString source;
    QString text;
    QByteArray payload;
    qint64 delayMs = 0;
};

/**
 * @brief Serial Station 回放计划。
 */
struct SerialReplayPlan {
    bool ok = false;
    QString errorMessage;
    QVector<SerialReplayEvent> events;
    qint64 totalDurationMs = 0;
    int skippedRecords = 0;
};

/**
 * @brief Serial Station 服务层日志回放计划生成器。
 */
class SerialReplayService {
public:
    /** @brief 从结构化日志生成回放计划。 */
    SerialReplayPlan buildPlan(const QVector<SerialLogRecord>& records,
                               const SerialReplayOptions& options = SerialReplayOptions()) const;

    /** @brief 从 JSON Lines 日志生成回放计划。 */
    SerialReplayPlan buildPlanFromJsonLines(
        const QString& jsonLines,
        const SerialReplayOptions& options = SerialReplayOptions()) const;

    /** @brief 生成单条回放事件摘要。 */
    QString eventSummary(const SerialReplayEvent& event) const;

    /** @brief 生成回放计划摘要。 */
    QString planSummary(const SerialReplayPlan& plan) const;

private:
    bool acceptsDirection(SerialLogDirection direction, const SerialReplayOptions& options) const;
    qint64 scaledDelayMs(qint64 rawDelayMs, const SerialReplayOptions& options) const;
    SerialReplayPlan makeFailure(const QString& message) const;
    SerialReplayPlan makePlan(const QVector<SerialLogRecord>& records,
                              const SerialReplayOptions& options) const;
    bool parseJsonLine(const QString& line,
                       int lineNumber,
                       SerialLogRecord& record,
                       QString& errorMessage) const;
    bool parseDirection(const QString& text,
                        SerialLogDirection& direction,
                        QString& errorMessage) const;
    bool parsePayloadHex(const QString& text, QByteArray& payload, QString& errorMessage) const;
};

} // namespace serial_station

#endif // SERIAL_REPLAY_SERVICE_H
