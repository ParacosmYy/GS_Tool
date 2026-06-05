/**
 * @file DataStreamMerger.cpp
 * @brief 多流数据合并器实现 -- RoundRobin/Priority/Timestamp/Fifo策略
 */

#include "utils/merger/DataStreamMerger.h"

#include <QDateTime>
#include <algorithm>

DataStreamMerger::DataStreamMerger(QObject* parent) : QObject(parent)
{
    setObjectName(QStringLiteral("DataStreamMerger"));
    m_timer.start();
}

void DataStreamMerger::setPolicy(MergePolicy p) { m_policy = p; }
DataStreamMerger::MergePolicy DataStreamMerger::policy() const { return m_policy; }

void DataStreamMerger::registerSource(const QString& name, int priority)
{
    if (m_streams.contains(name)) return;
    m_streams[name] = QList<StreamEntry>();
    m_priorities[name] = priority;
    m_rrOrder.append(name);
    ++m_stats.totalSourcesRegistered;
    emit sourceRegistered(name);
}

void DataStreamMerger::removeSource(const QString& name)
{
    if (!m_streams.contains(name)) return;
    m_streams.remove(name);
    m_priorities.remove(name);
    m_rrOrder.removeOne(name);
    if (m_rrIndex >= m_rrOrder.size()) m_rrIndex = 0;
    ++m_stats.totalSourcesRemoved;
    emit sourceRemoved(name);
}

void DataStreamMerger::feed(const QString& sourceName, const QByteArray& data)
{
    if (!m_streams.contains(sourceName)) registerSource(sourceName);
    StreamEntry entry;
    entry.data = data;
    entry.sourceName = sourceName;
    entry.timestampMs = QDateTime::currentMSecsSinceEpoch();
    entry.priority = m_priorities.value(sourceName, 0);
    m_streams[sourceName].append(entry);
}

DataStreamMerger::StreamEntry DataStreamMerger::mergeNext()
{
    ++m_stats.mergeCycles;
    StreamEntry result;
    switch (m_policy) {
    case MergePolicy::RoundRobin:    result = mergeRoundRobin(); break;
    case MergePolicy::Priority:      result = mergePriority(); break;
    case MergePolicy::TimestampOrder: result = mergeTimestamp(); break;
    case MergePolicy::Fifo:          result = mergeFifo(); break;
    }
    if (!result.data.isEmpty()) {
        ++m_stats.totalEntriesMerged;
        m_stats.totalBytesMerged += static_cast<quint64>(result.data.size());
        emit entryMerged(result);
    }
    return result;
}

QList<DataStreamMerger::StreamEntry> DataStreamMerger::mergeAll()
{
    QList<StreamEntry> results;
    while (hasData()) results.append(mergeNext());
    return results;
}

DataStreamMerger::StreamEntry DataStreamMerger::mergeRoundRobin()
{
    int attempts = m_rrOrder.size();
    while (attempts-- > 0) {
        if (m_rrIndex >= m_rrOrder.size()) m_rrIndex = 0;
        const QString& name = m_rrOrder[m_rrIndex];
        ++m_rrIndex;
        if (m_streams.contains(name) && !m_streams[name].isEmpty())
            return m_streams[name].takeFirst();
        ++m_stats.starvationEvents;
        emit sourceStarved(name);
    }
    return {};
}

DataStreamMerger::StreamEntry DataStreamMerger::mergePriority()
{
    QString bestSource;
    int bestPri = INT_MIN;
    for (auto it = m_streams.constBegin(); it != m_streams.constEnd(); ++it) {
        if (!it.value().isEmpty()) {
            int pri = m_priorities.value(it.key(), 0);
            if (pri > bestPri) { bestPri = pri; bestSource = it.key(); }
        }
    }
    if (bestSource.isEmpty()) return {};
    return m_streams[bestSource].takeFirst();
}

DataStreamMerger::StreamEntry DataStreamMerger::mergeTimestamp()
{
    QString bestSource;
    qint64 bestTs = LLONG_MAX;
    for (auto it = m_streams.constBegin(); it != m_streams.constEnd(); ++it) {
        if (!it.value().isEmpty() && it.value().first().timestampMs < bestTs) {
            bestTs = it.value().first().timestampMs;
            bestSource = it.key();
        }
    }
    if (bestSource.isEmpty()) return {};
    return m_streams[bestSource].takeFirst();
}

DataStreamMerger::StreamEntry DataStreamMerger::mergeFifo()
{
    qint64 bestTs = LLONG_MAX;
    QString bestSource;
    for (auto it = m_streams.constBegin(); it != m_streams.constEnd(); ++it) {
        if (!it.value().isEmpty()) {
            /* FIFO: 使用feed顺序（队首最早） */
            bestSource = it.key();
            break;
        }
    }
    if (bestSource.isEmpty()) return {};
    return m_streams[bestSource].takeFirst();
}

bool DataStreamMerger::hasData() const
{
    for (auto it = m_streams.constBegin(); it != m_streams.constEnd(); ++it)
        if (!it.value().isEmpty()) return true;
    return false;
}

int DataStreamMerger::pendingCount() const
{
    int count = 0;
    for (auto it = m_streams.constBegin(); it != m_streams.constEnd(); ++it)
        count += it.value().size();
    return count;
}

int DataStreamMerger::sourceCount() const { return m_streams.size(); }

void DataStreamMerger::clear()
{
    for (auto it = m_streams.begin(); it != m_streams.end(); ++it) it.value().clear();
}

DataStreamMerger::Stats DataStreamMerger::stats() const { return m_stats; }
void DataStreamMerger::resetStatistics() { m_stats = Stats{}; }
