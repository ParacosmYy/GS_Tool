/**
 * @file RecordingMarker.cpp
 * @brief 录制标记管理器实现
 */

#include "core/recording/RecordingMarker.h"

RecordingMarker::RecordingMarker(QObject* parent)
    : QObject(parent)
{
}

void RecordingMarker::addMarker(const QString& label, qint64 timestampMs)
{
    MarkerEntry entry;
    entry.label = label;
    entry.timestampMs = timestampMs;
    m_markers.append(entry);
    emit markerAdded(m_markers.size() - 1, label);
}

void RecordingMarker::removeMarker(int index)
{
    if (index < 0 || index >= m_markers.size()) {
        return;
    }
    m_markers.removeAt(index);
    emit markerRemoved(index);
}

QList<MarkerEntry> RecordingMarker::markers() const
{
    return m_markers;
}
