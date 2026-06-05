/**
 * @file ProtocolDuplicatorIO.cpp
 * @brief 协议流量复制器 -- JSON序列化/导出导入/统计/私有方法
 *
 * 从 ProtocolDuplicator.cpp 拆分: recordingToJson/recordingFromJson/
 * exportRecording/importRecording/stats + 所有私有槽和辅助方法。
 */

#include "protocol/duplicate/ProtocolDuplicator.h"

#include <QJsonDocument>
#include <QFile>
#include <QDateTime>
#include <algorithm>
#include <numeric>

// ============================================================================
// JSON序列化
// ============================================================================

/**
 * @brief 录制转JSON对象
 * @param recording 要序列化的录制
 * @return QJsonObject 完整的录制数据
 */
QJsonObject ProtocolDuplicator::recordingToJson(const Recording& recording) const
{
    QJsonObject root;
    root["id"] = recording.id;
    root["label"] = recording.label;
    root["startTimestampMs"] = recording.startTimestampMs;
    root["endTimestampMs"] = recording.endTimestampMs;
    root["sourceName"] = recording.sourceName;
    root["targetName"] = recording.targetName;

    QJsonArray entriesArr;
    for (const TrafficEntry& entry : recording.entries) {
        QJsonObject entryObj;
        entryObj["direction"] = static_cast<int>(entry.direction);
        entryObj["data"] = QString::fromUtf8(entry.data.toHex());
        entryObj["timestampMs"] = entry.timestampMs;
        entryObj["relativeOffsetMs"] = entry.relativeOffsetMs;
        entryObj["sequenceIndex"] = static_cast<qint64>(entry.sequenceIndex);
        entriesArr.append(entryObj);
    }
    root["entries"] = entriesArr;
    return root;
}

/** @brief JSON对象转录制 @param obj JSON对象 @return 反序列化的录制 */
ProtocolDuplicator::Recording ProtocolDuplicator::recordingFromJson(const QJsonObject& obj) const
{
    Recording rec;
    rec.id = obj["id"].toString();
    rec.label = obj["label"].toString();
    rec.startTimestampMs = obj["startTimestampMs"].toInteger();
    rec.endTimestampMs = obj["endTimestampMs"].toInteger();
    rec.sourceName = obj["sourceName"].toString();
    rec.targetName = obj["targetName"].toString();

    QJsonArray entriesArr = obj["entries"].toArray();
    for (const QJsonValue& val : entriesArr) {
        QJsonObject entryObj = val.toObject();
        TrafficEntry entry;
        entry.direction = static_cast<Direction>(entryObj["direction"].toInt(0));
        entry.data = QByteArray::fromHex(entryObj["data"].toString().toUtf8());
        entry.timestampMs = entryObj["timestampMs"].toInteger();
        entry.relativeOffsetMs = entryObj["relativeOffsetMs"].toInteger();
        entry.sequenceIndex = entryObj["sequenceIndex"].toInteger();
        rec.entries.append(entry);
    }
    return rec;
}

// ============================================================================
// 文件导出 / 导入
// ============================================================================

/** @brief 导出录制到文件 @return true=成功 */
bool ProtocolDuplicator::exportRecording(const Recording& recording,
                                         const QString& filePath) const
{
    QJsonDocument doc(recordingToJson(recording));
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return false;
    }
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    return true;
}

/** @brief 从文件导入录制 @return 反序列化的录制 */
ProtocolDuplicator::Recording ProtocolDuplicator::importRecording(const QString& filePath) const
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return Recording();
    }
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    if (doc.isNull() || !doc.isObject()) {
        return Recording();
    }
    return recordingFromJson(doc.object());
}

// ============================================================================
// 统计
// ============================================================================

const ProtocolDuplicator::DuplicationStats& ProtocolDuplicator::stats() const
{
    return m_stats;
}

// ============================================================================
// 私有方法
// ============================================================================

/** @brief 源连接收到数据回调(RX方向) */
void ProtocolDuplicator::onSourceDataReceived(const QByteArray& data)
{
    if (!m_recording || data.isEmpty()) {
        return;
    }

    TrafficEntry entry;
    entry.direction = Rx;
    entry.data = data;
    entry.timestampMs = QDateTime::currentMSecsSinceEpoch();
    entry.relativeOffsetMs = static_cast<qint64>(m_recordTimer.elapsed());
    entry.sequenceIndex = static_cast<qint64>(m_currentRecording.entries.size());
    m_currentRecording.entries.append(entry);
}

/** @brief 源连接数据已发送回调(TX方向) */
void ProtocolDuplicator::onSourceBytesWritten(qint64 bytes)
{
    Q_UNUSED(bytes)
}

/**
 * @brief 对数据应用修改规则
 * @param data 原始数据
 * @param entryIndex 当前条目索引
 * @return 修改后的数据
 */
QByteArray ProtocolDuplicator::applyModifications(const QByteArray& data,
                                                   qint64 entryIndex) const
{
    QByteArray result = data;

    for (const ModificationRule& rule : m_modificationRules) {
        if (rule.entryIndex >= 0 && rule.entryIndex != entryIndex) {
            continue;
        }
        if (rule.byteOffset < 0 ||
            rule.byteOffset + rule.newValue.size() > result.size()) {
            continue;
        }
        if (!rule.oldValue.isEmpty()) {
            QByteArray original = result.mid(rule.byteOffset, rule.oldValue.size());
            if (original != rule.oldValue) {
                continue;
            }
        }
        result.replace(rule.byteOffset, rule.newValue.size(), rule.newValue);
    }

    return result;
}

/** @brief 调度下一个回放条目 */
void ProtocolDuplicator::scheduleNextEntry()
{
    if (!m_duplicating) {
        return;
    }

    const auto& entries = m_playbackRecording.entries;

    if (m_playbackEntryIndex >= static_cast<qint64>(entries.size())) {
        m_currentRepeatIndex++;
        m_stats.duplicationCount++;

        bool shouldContinue = false;
        switch (m_scheduleMode) {
        case Once:
            shouldContinue = false;
            break;
        case RepeatN:
            shouldContinue = (m_currentRepeatIndex < m_repeatCount);
            break;
        case TimedInterval:
            shouldContinue = true;
            break;
        case ContinuousLoop:
            shouldContinue = true;
            break;
        }

        if (!shouldContinue) {
            m_duplicating = false;
            emit duplicationComplete();
            return;
        }

        m_playbackEntryIndex = 0;
    }

    qint64 delayMs = 0;
    if (m_intervalMs > 0) {
        delayMs = m_intervalMs;
    } else if (m_playbackEntryIndex > 0) {
        delayMs = entries[m_playbackEntryIndex].relativeOffsetMs
                - entries[m_playbackEntryIndex - 1].relativeOffsetMs;
        delayMs = qMax(static_cast<qint64>(0), delayMs);
    }

    if (delayMs > 0) {
        m_delaySamples.append(static_cast<double>(delayMs));
        if (m_delaySamples.size() > 500) {
            m_delaySamples.removeFirst();
        }
        double sum = std::accumulate(m_delaySamples.begin(), m_delaySamples.end(), 0.0);
        m_stats.avgDelayMs = sum / static_cast<double>(m_delaySamples.size());
    }

    if (delayMs > 0) {
        m_playbackTimer->start(static_cast<int>(delayMs));
    } else {
        sendEntry(entries[m_playbackEntryIndex]);
        m_playbackEntryIndex++;
        scheduleNextEntry();
    }
}

/** @brief 发送单条流量条目到目标连接 */
void ProtocolDuplicator::sendEntry(const TrafficEntry& entry)
{
    if (!m_target || entry.data.isEmpty()) {
        return;
    }

    if (entry.direction == Tx) {
        QByteArray modified = applyModifications(entry.data, m_playbackEntryIndex);
        m_target->write(modified);
        m_stats.totalBytesDuplicated += static_cast<quint64>(modified.size());
    }

    m_stats.totalDuplicated++;
}

/** @brief 连接源信号 */
void ProtocolDuplicator::connectSourceSignals()
{
    if (!m_source) {
        return;
    }
    connect(m_source, &IConnection::dataReceived,
            this, &ProtocolDuplicator::onSourceDataReceived,
            Qt::UniqueConnection);
    connect(m_source, &IConnection::bytesWritten,
            this, &ProtocolDuplicator::onSourceBytesWritten,
            Qt::UniqueConnection);
}

/** @brief 断开源信号 */
void ProtocolDuplicator::disconnectSourceSignals()
{
    if (!m_source) {
        return;
    }
    disconnect(m_source, &IConnection::dataReceived,
               this, &ProtocolDuplicator::onSourceDataReceived);
    disconnect(m_source, &IConnection::bytesWritten,
               this, &ProtocolDuplicator::onSourceBytesWritten);
}
