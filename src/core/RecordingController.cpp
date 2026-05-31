#include "RecordingController.h"
#include "utils/DataLogger.h"
#include <QToolBar>
#include <QFileDialog>
#include <QMessageBox>

RecordingController::RecordingController(DataLogger* logger, QObject* parent)
    : QObject(parent)
    , m_logger(logger)
{
    // DataLogger信号 → RecordingController槽
    connect(m_logger, &DataLogger::playbackData,
            this, &RecordingController::onPlaybackData);
    connect(m_logger, &DataLogger::playbackProgress,
            this, &RecordingController::onPlaybackProgress);
    connect(m_logger, &DataLogger::recordingStopped,
            this, &RecordingController::onRecordingStopped);
    connect(m_logger, &DataLogger::playbackFinished, this, [this]() {
        m_stopPlaybackAction->setEnabled(false);
        m_playbackAction->setEnabled(true);
        emit statusMessage(tr("Playback finished"), 3000);
    });
    connect(m_logger, &DataLogger::error, this, [this](const QString& msg) {
        emit statusMessage(msg, 5000);
    });
}

void RecordingController::setupActions(QToolBar* toolbar)
{
    // 日志录制按钮
    m_recordAction = toolbar->addAction(tr("录制"));
    m_recordAction->setCheckable(true);
    m_recordAction->setChecked(false);

    m_stopRecordAction = toolbar->addAction(tr("停止录制"));
    m_stopRecordAction->setEnabled(false);

    // 日志回放按钮
    m_playbackAction = toolbar->addAction(tr("回放日志"));
    m_stopPlaybackAction = toolbar->addAction(tr("停止回放"));
    m_stopPlaybackAction->setEnabled(false);

    // 按钮信号 → 槽
    connect(m_recordAction, &QAction::toggled,
            this, &RecordingController::onToggleRecording);
    connect(m_stopRecordAction, &QAction::triggered,
            this, &RecordingController::onStopRecording);
    connect(m_playbackAction, &QAction::triggered,
            this, &RecordingController::onOpenPlayback);
    connect(m_stopPlaybackAction, &QAction::triggered,
            this, &RecordingController::onStopPlayback);
}

void RecordingController::setConnected(bool connected)
{
    m_connected = connected;
}

void RecordingController::onToggleRecording()
{
    if (m_logger->isRecording()) {
        // 正在录制 → 暂停/恢复切换
        if (m_logger->isPaused()) {
            m_logger->resumeRecording();
            m_recordAction->setText(tr("暂停"));
        } else {
            m_logger->pauseRecording();
            m_recordAction->setText(tr("继续"));
        }
    } else {
        // 开始录制
        QString filter = tr("EmbedDebug Log (*.edl);;All files (*.*)");
        QString path = QFileDialog::getSaveFileName(
            nullptr, tr("录制日志"), QString(), filter);
        if (path.isEmpty()) {
            m_recordAction->setChecked(false);
            return;
        }
        if (!path.endsWith(".edl")) path += ".edl";

        m_logger->startRecording(path);
        m_stopRecordAction->setEnabled(true);
        m_recordAction->setText(tr("暂停"));
        emit statusMessage(tr("Recording: %1").arg(path));
    }
}

void RecordingController::onStopRecording()
{
    m_logger->stopRecording();
    m_recordAction->setChecked(false);
    m_recordAction->setText(tr("录制"));
    m_stopRecordAction->setEnabled(false);
}

void RecordingController::onOpenPlayback()
{
    QString filter = tr("EmbedDebug Log (*.edl);;All files (*.*)");
    QString path = QFileDialog::getOpenFileName(
        nullptr, tr("打开日志回放"), QString(), filter);
    if (path.isEmpty()) return;

    m_logger->startPlayback(path);
    m_stopPlaybackAction->setEnabled(true);
    m_playbackAction->setEnabled(false);
    emit statusMessage(tr("Playing: %1").arg(path));
}

void RecordingController::onStopPlayback()
{
    m_logger->stopPlayback();
    m_stopPlaybackAction->setEnabled(false);
    m_playbackAction->setEnabled(true);
}

void RecordingController::onPlaybackData(const QByteArray& data, qint64 direction)
{
    // 转发给MainWindow处理终端显示
    emit playbackData(data, direction);
}

void RecordingController::onPlaybackProgress(qreal percent)
{
    emit statusMessage(tr("Playback: %1%").arg(static_cast<int>(percent * 100)));
}

void RecordingController::onRecordingStopped(const QString& filePath, int count, qint64 durationMs)
{
    emit statusMessage(
        tr("Recording saved: %1 (%2 records, %3s)")
            .arg(filePath)
            .arg(count)
            .arg(durationMs / 1000.0, 0, 'f', 1),
        5000);
}
