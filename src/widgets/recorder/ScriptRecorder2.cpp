/**
 * @file ScriptRecorder2.cpp
 * @brief 脚本录制器v2实现 — 操作录制/回放/速度控制
 */
#include "widgets/recorder/ScriptRecorder2.h"
#include <QDateTime>

/** @brief 构造函数 @param parent 父Widget */
ScriptRecorder::ScriptRecorder(QWidget *parent) : QWidget(parent), m_timer(new QTimer(this)) { setObjectName("ScriptRecorder2"); connect(m_timer, &QTimer::timeout, this, &ScriptRecorder::onPlaybackTick); }
/** @brief 析构函数 */
ScriptRecorder::~ScriptRecorder() = default;
/** @brief 开始录制，清空历史条目 */
void ScriptRecorder::startRecording() { m_entries.clear(); m_recording = true; emit recordingStarted(); }
/** @brief 停止录制 @param parent (未使用) */
void ScriptRecorder::stopRecording() { m_recording = false; emit recordingStopped(m_entries.size()); }
/** @brief 查询是否正在录制 @return 录制中返回true */
bool ScriptRecorder::isRecording() const { return m_recording; }
/** @brief 录制一个操作条目 @param action 操作名称 @param data 操作数据 */
void ScriptRecorder::recordAction(const QString &action, const QByteArray &data) { if (m_recording) { ++m_totalRecords; m_entries.append({QDateTime::currentMSecsSinceEpoch(), action, data}); emit actionRecorded(action); } }
/** @brief 开始回放录制的操作序列 */
void ScriptRecorder::playback() { if (m_entries.isEmpty()) return; m_playing = true; m_playIndex = 0; emit playbackStarted(); m_timer->start(10); }
/** @brief 清空所有录制条目 */
void ScriptRecorder::clear() { m_entries.clear(); }
/** @brief 获取所有录制条目 @return 条目列表 */
QList<ScriptRecorder::RecordEntry> ScriptRecorder::entries() const { return m_entries; }
/** @brief 获取录制条目数量 @return 条目数 */
int ScriptRecorder::entryCount() const { return m_entries.size(); }
/** @brief 设置回放速度倍率 @param s 速度倍率 */
void ScriptRecorder::setPlaybackSpeed(double s) { m_speed = s; }

/** @brief 回放定时器回调 — 逐条发射操作信号 */
void ScriptRecorder::onPlaybackTick() {
    if (m_playIndex >= m_entries.size()) { m_timer->stop(); m_playing = false; emit playbackFinished(); return; }
    const auto &e = m_entries[m_playIndex];
    emit playbackAction(e.action, e.data);
    m_playIndex++;
}
