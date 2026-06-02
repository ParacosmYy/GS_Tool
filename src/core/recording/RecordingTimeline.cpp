/**
 * @file RecordingTimeline.cpp
 * @brief 录制时间线管理器实现
 */

#include "core/recording/RecordingTimeline.h"
#include <QDateTime>

RecordingTimeline::RecordingTimeline(QObject* parent)
    : QObject(parent)
    , m_startTimeMs(0)
    , m_elapsedMs(0)
    , m_recording(false)
{
}

void RecordingTimeline::startRecording()
{
    m_startTimeMs = QDateTime::currentMSecsSinceEpoch();
    m_elapsedMs = 0;
    m_recording = true;
    emit recordingStarted();
}

void RecordingTimeline::stopRecording()
{
    if (!m_recording) {
        return;
    }
    m_elapsedMs = QDateTime::currentMSecsSinceEpoch() - m_startTimeMs;
    m_recording = false;
    emit recordingStopped(m_elapsedMs);
}

qint64 RecordingTimeline::currentTimeMs() const
{
    return m_elapsedMs;
}

void RecordingTimeline::seekTo(qint64 timeMs)
{
    m_elapsedMs = timeMs;
    emit timeUpdated(m_elapsedMs);
}

QList<qint64> RecordingTimeline::eventTimestamps() const
{
    // TODO: 返回已记录的事件时间戳列表
    return QList<qint64>();
}
