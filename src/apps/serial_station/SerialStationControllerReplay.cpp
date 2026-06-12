#include "apps/serial_station/SerialStationController.h"

namespace serial_station {

namespace {

constexpr int kMaxPreviewEvents = 5;

QString replayDirectionText(SerialLogDirection direction)
{
    return SerialLogService::directionName(direction).toUpper();
}

QString replayEventText(const SerialReplayEvent& event)
{
    QString text = QStringLiteral("+%1ms %2 %3")
                       .arg(QString::number(event.delayMs),
                            replayDirectionText(event.direction),
                            event.text);

    const QString payloadHex = SerialLogService::payloadHex(event.payload);
    if (!payloadHex.isEmpty()) {
        text.append(QStringLiteral(" hex=%1").arg(payloadHex));
    }

    return text;
}

} // namespace

SerialReplayPlan SerialStationController::previewReplayPlan(const SerialReplayOptions& options)
{
    const SerialReplayPlan plan = m_replayService.buildPlan(m_logService.records(), options);
    if (!plan.ok) {
        logError(tr("回放预览失败: %1").arg(plan.errorMessage),
                 {{QStringLiteral("message"), plan.errorMessage}});
        return plan;
    }

    logSystem(tr("回放预览: %1 个事件, %2 ms, 跳过 %3 条")
                  .arg(QString::number(plan.events.count()),
                       QString::number(plan.totalDurationMs),
                       QString::number(plan.skippedRecords)),
              {{QStringLiteral("events"), plan.events.count()},
               {QStringLiteral("totalDurationMs"), plan.totalDurationMs},
               {QStringLiteral("skippedRecords"), plan.skippedRecords}});

    const int previewCount = qMin(kMaxPreviewEvents, plan.events.count());
    for (int index = 0; index < previewCount; ++index) {
        logSystem(tr("回放事件 %1/%2: %3")
                      .arg(QString::number(index + 1),
                           QString::number(plan.events.count()),
                           replayEventText(plan.events.at(index))),
                  {{QStringLiteral("eventIndex"), index},
                   {QStringLiteral("eventCount"), plan.events.count()},
                   {QStringLiteral("delayMs"), plan.events.at(index).delayMs}});
    }

    if (plan.events.count() > previewCount) {
        logSystem(tr("回放预览已截断: 还剩 %1 个事件")
                      .arg(QString::number(plan.events.count() - previewCount)),
                  {{QStringLiteral("remainingEvents"), plan.events.count() - previewCount}});
    }

    return plan;
}

} // namespace serial_station
