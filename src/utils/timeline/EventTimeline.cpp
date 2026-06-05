/**
 * @file EventTimeline.cpp
 * @brief 事件时间线实现 — 时间戳事件记录与查询
 */

#include "utils/timeline/EventTimeline.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDateTime>
#include <QMutexLocker>

/** @brief 构造函数 @param parent 父对象 */
EventTimeline::EventTimeline(QObject* parent)
    : QObject(parent)
    , m_maxEvents(100000)
{
}

/** @brief 记录事件 @param category 类别 @param severity 级别 @param message 消息 */
void EventTimeline::record(Category category, Severity severity,
                           const QString& message)
{
    Event event;
    event.timestamp = QDateTime::currentMSecsSinceEpoch();
    event.category = category;
    event.severity = severity;
    event.message = message;

    m_events.append(event);

    /* 保留上限 */
    while (m_events.size() > m_maxEvents) {
        m_events.removeFirst();
    }

    /* 更新统计 */
    ++m_stats.totalEvents;
    int catKey = static_cast<int>(category);
    int sevKey = static_cast<int>(severity);
    ++m_stats.eventsByCategory[catKey];
    ++m_stats.eventsBySeverity[sevKey];

    /* 计算每秒事件峰值 */
    qint64 sec = event.timestamp / 1000;
    ++m_eventsPerSecond[sec];
    double rate = static_cast<double>(m_eventsPerSecond[sec]);
    if (rate > m_stats.peakEventsPerSecond) {
        m_stats.peakEventsPerSecond = rate;
    }

    /* 清理旧的秒级计数 */
    qint64 cutoff = sec - 60;
    auto it = m_eventsPerSecond.begin();
    while (it != m_eventsPerSecond.end() && it.key() < cutoff) {
        it = m_eventsPerSecond.erase(it);
    }

    emit eventRecorded(event);

    if (severity == Severity::Critical) {
        emit criticalEvent(event);
    }
}

/** @brief 记录带元数据的事件 */
void EventTimeline::recordWithMetadata(
    Category category, Severity severity, const QString& message,
    const QMap<QString, QString>& metadata)
{
    record(category, severity, message);
    if (!m_events.isEmpty()) {
        m_events.last().metadata = metadata;
    }
}

/** @brief 按时间范围查询 @param fromMs 起始 @param toMs 结束 @return 事件列表 */
QList<EventTimeline::Event> EventTimeline::queryByTime(
    qint64 fromMs, qint64 toMs) const
{
    QList<Event> result;
    for (const auto& event : m_events) {
        if (event.timestamp >= fromMs && event.timestamp <= toMs) {
            result.append(event);
        }
    }
    return result;
}

/** @brief 按类别查询 @param category 类别 @return 事件列表 */
QList<EventTimeline::Event> EventTimeline::queryByCategory(
    Category category) const
{
    QList<Event> result;
    int key = static_cast<int>(category);
    for (const auto& event : m_events) {
        if (static_cast<int>(event.category) == key) {
            result.append(event);
        }
    }
    return result;
}

/** @brief 按严重级别查询 @param severity 级别 @return 事件列表 */
QList<EventTimeline::Event> EventTimeline::queryBySeverity(
    Severity severity) const
{
    QList<Event> result;
    int key = static_cast<int>(severity);
    for (const auto& event : m_events) {
        if (static_cast<int>(event.severity) == key) {
            result.append(event);
        }
    }
    return result;
}

/** @brief 查找时间戳附近的事件 @param ts 时间戳 @param maxDelta 最大偏差 @return 事件列表 */
QList<EventTimeline::Event> EventTimeline::findNear(
    qint64 ts, qint64 maxDelta) const
{
    QList<Event> result;
    for (const auto& event : m_events) {
        if (qAbs(event.timestamp - ts) <= maxDelta) {
            result.append(event);
        }
    }
    return result;
}

/** @brief 导出为JSON @param path 文件路径 @return 是否成功 */
bool EventTimeline::exportToJson(const QString& path) const
{
    QJsonArray arr;
    for (const auto& event : m_events) {
        QJsonObject obj;
        obj["timestamp"] = static_cast<qint64>(event.timestamp);
        obj["datetime"] = QDateTime::fromMSecsSinceEpoch(event.timestamp)
            .toString(Qt::ISODateWithMs);
        obj["category"] = static_cast<int>(event.category);
        obj["severity"] = static_cast<int>(event.severity);
        obj["message"] = event.message;

        QJsonObject meta;
        for (auto it = event.metadata.constBegin();
             it != event.metadata.constEnd(); ++it) {
            meta[it.key()] = it.value();
        }
        obj["metadata"] = meta;

        arr.append(obj);
    }

    QJsonDocument doc(arr);
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    file.write(doc.toJson(QJsonDocument::Indented));
    return true;
}

/** @brief 清除所有事件 */
void EventTimeline::clear()
{
    m_events.clear();
    m_eventsPerSecond.clear();
}

/** @brief 重置统计 */
void EventTimeline::resetStatistics()
{
    m_stats = Stats{};
    m_eventsPerSecond.clear();
}
