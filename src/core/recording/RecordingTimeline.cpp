/**
 * @file RecordingTimeline.cpp
 * @brief 录制时间线管理器实现
 *
 * 管理录制会话的完整生命周期，包括启停控制、实时时间追踪、
 * 事件时间戳记录与回放定位功能。
 */

#include "core/recording/RecordingTimeline.h"
#include <QDateTime>

/** @brief 构造函数，初始化所有时间成员为零，录制状态为false @param parent 父对象指针 */
RecordingTimeline::RecordingTimeline(QObject* parent)
    : QObject(parent)
    , m_startTimeMs(0)
    , m_elapsedMs(0)
    , m_seekPositionMs(0)
    , m_recording(false)
{
}

/** @brief 开始录制，重置时间基准并进入录制状态 */
void RecordingTimeline::startRecording()
{
    m_startTimeMs = QDateTime::currentMSecsSinceEpoch();
    m_elapsedMs = 0;
    m_recording = true;
    m_events.clear();
    ++m_totalRecordingStarts;
    emit recordingStarted();
}

/** @brief 停止录制，计算总时长并自动记录停止时刻为事件 */
void RecordingTimeline::stopRecording()
{
    if (!m_recording) {
        return;
    }
    m_elapsedMs = QDateTime::currentMSecsSinceEpoch() - m_startTimeMs;
    m_recording = false;
    ++m_totalRecordingStops;

    // 自动记录停止时刻作为事件
    m_events.append(m_elapsedMs);

    emit recordingStopped(m_elapsedMs);
}

/** @brief 获取当前已录制时长(录制中返回实时差值，未录制返回回放定位位置) @return 当前时间点(毫秒) */
qint64 RecordingTimeline::currentTimeMs() const
{
    if (m_recording) {
        return QDateTime::currentMSecsSinceEpoch() - m_startTimeMs;
    }
    return m_seekPositionMs;
}

/** @brief 定位到指定时间点(非录制状态下有效，自动钳制到有效范围) @param timeMs 目标时间点(毫秒)，负值会被钳制为0 */
void RecordingTimeline::seekTo(qint64 timeMs)
{
    if (m_recording) {
        return;
    }

    // 钳制到有效范围 [0, m_elapsedMs]
    if (timeMs < 0) {
        timeMs = 0;
    } else if (timeMs > m_elapsedMs) {
        timeMs = m_elapsedMs;
    }

    m_seekPositionMs = timeMs;
    ++m_totalSeeks;
    emit timeUpdated(m_seekPositionMs);
}

/** @brief 获取所有事件时间戳(按记录顺序排列) @return 事件时间戳列表(毫秒) */
QList<qint64> RecordingTimeline::eventTimestamps() const
{
    return m_events;
}

/** @brief 记录一个事件时间戳(非负且不超过当前录制时长才接受) @param timestampMs 事件发生的时间点(毫秒) */
void RecordingTimeline::recordEvent(qint64 timestampMs)
{
    // 校验：时间戳非负且不超过当前录制时长
    if (timestampMs < 0) {
        return;
    }
    if (timestampMs > currentTimeMs()) {
        return;
    }

    m_events.append(timestampMs);
    ++m_totalEventsRecorded;
}

/** @brief 清除所有已记录的事件时间戳(不影响录制状态和时间计数) */
void RecordingTimeline::clearEvents()
{
    m_events.clear();
}

/** @brief 获取录制总时长(录制中返回当前已过时长，未录制返回累计时长) @return 录制总时长(毫秒) */
qint64 RecordingTimeline::totalDurationMs() const
{
    return currentTimeMs();
}
