#include "widgets/recorder/ScriptRecorder2.h"
#include <QDateTime>
ScriptRecorder::ScriptRecorder(QWidget *parent) : QWidget(parent), m_timer(new QTimer(this)) { setObjectName("ScriptRecorder2"); connect(m_timer, &QTimer::timeout, this, &ScriptRecorder::onPlaybackTick); }
ScriptRecorder::~ScriptRecorder() = default;
void ScriptRecorder::startRecording() { m_entries.clear(); m_recording = true; emit recordingStarted(); }
void ScriptRecorder::stopRecording() { m_recording = false; emit recordingStopped(m_entries.size()); }
bool ScriptRecorder::isRecording() const { return m_recording; }
void ScriptRecorder::recordAction(const QString &action, const QByteArray &data) { if (m_recording) { m_entries.append({QDateTime::currentMSecsSinceEpoch(), action, data}); emit actionRecorded(action); } }
void ScriptRecorder::playback() { if (m_entries.isEmpty()) return; m_playing = true; m_playIndex = 0; emit playbackStarted(); m_timer->start(10); }
void ScriptRecorder::clear() { m_entries.clear(); }
QList<ScriptRecorder::RecordEntry> ScriptRecorder::entries() const { return m_entries; }
int ScriptRecorder::entryCount() const { return m_entries.size(); }
void ScriptRecorder::setPlaybackSpeed(double s) { m_speed = s; }
void ScriptRecorder::onPlaybackTick() {
    if (m_playIndex >= m_entries.size()) { m_timer->stop(); m_playing = false; emit playbackFinished(); return; }
    const auto &e = m_entries[m_playIndex];
    emit playbackAction(e.action, e.data);
    m_playIndex++;
}
