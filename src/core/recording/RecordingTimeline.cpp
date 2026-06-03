/**
 * @file RecordingTimeline.cpp
 * @brief 录制时间线管理器实现
 *
 * 管理录制会话的完整生命周期，包括启停控制、实时时间追踪、
 * 事件时间戳记录与回放定位功能。
 */

#include "core/recording/RecordingTimeline.h"
#include <QDateTime>

/**
 * @brief 构造函数
 *
 * 初始化所有时间成员为零，录制状态为 false。
 *
 * @param parent 父对象指针，用于 Qt 对象树管理
 */
RecordingTimeline::RecordingTimeline(QObject* parent)
    : QObject(parent)
    , m_startTimeMs(0)
    , m_elapsedMs(0)
    , m_seekPositionMs(0)
    , m_recording(false)
{
}

/**
 * @brief 开始录制
 *
 * 重置时间基准为当前绝对时间，清空上次录制时长和事件列表，
 * 进入录制状态并发出 recordingStarted() 信号。
 */
void RecordingTimeline::startRecording()
{
    m_startTimeMs = QDateTime::currentMSecsSinceEpoch();
    m_elapsedMs = 0;
    m_recording = true;
    m_events.clear();
    ++m_totalRecordingStarts;
    emit recordingStarted();
}

/**
 * @brief 停止录制
 *
 * 计算本次录制的总时长并缓存到 m_elapsedMs，退出录制状态。
 * 自动将停止时刻记录为一个事件时间戳，然后发出 recordingStopped() 信号。
 * 若当前未在录制则直接返回。
 */
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

/**
 * @brief 获取当前已录制时长
 *
 * 录制中时返回从 m_startTimeMs 到当前时刻的实时差值（毫秒），
 * 未录制时返回上次录制的累计时长 m_elapsedMs。
 *
 * @return 当前时间点（毫秒）
 */
qint64 RecordingTimeline::currentTimeMs() const
{
    if (m_recording) {
        return QDateTime::currentMSecsSinceEpoch() - m_startTimeMs;
    }
    return m_seekPositionMs;
}

/**
 * @brief 定位到指定时间点
 *
 * 仅在非录制状态下有效。将内部时间指针移动到目标位置，
 * 目标值会被钳制在 [0, m_elapsedMs] 范围内。
 * 定位完成后发出 timeUpdated() 信号。
 *
 * @param timeMs 目标时间点（毫秒），负值会被钳制为 0
 */
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

/**
 * @brief 获取所有事件时间戳
 *
 * 返回录制过程中通过 recordEvent() 和 stopRecording() 记录的
 * 所有事件时间点，按记录顺序排列。
 *
 * @return 事件时间戳列表（毫秒）
 */
QList<qint64> RecordingTimeline::eventTimestamps() const
{
    return m_events;
}

/**
 * @brief 记录一个事件时间戳
 *
 * 将指定时间戳追加到事件列表末尾。仅当时间戳非负且不超过
 * 当前已录制时长时才会被接受。
 *
 * @param timestampMs 事件发生的时间点（毫秒）
 */
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

/**
 * @brief 清除所有已记录的事件时间戳
 *
 * 清空事件列表，不影响录制状态和时间计数。
 */
void RecordingTimeline::clearEvents()
{
    m_events.clear();
}

/**
 * @brief 获取录制总时长
 *
 * 未录制时返回上次录制结束的累计时长；录制中返回当前已过时长。
 * 与 currentTimeMs() 语义一致，但命名更清晰地表达"总时长"意图。
 *
 * @return 录制总时长（毫秒）
 */
qint64 RecordingTimeline::totalDurationMs() const
{
    return currentTimeMs();
}
