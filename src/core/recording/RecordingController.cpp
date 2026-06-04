/**
 * @file RecordingController.cpp
 * @brief 录制/回放控制器实现 - 管理录制/回放按钮的交互和数据流转发
 */

#include "core/recording/RecordingController.h"
#include "utils/log/DataLogger.h"
#include <QToolBar>
#include <QFileDialog>

/** @brief 构造录制控制器，连接DataLogger信号到内部槽建立数据流管道 @param logger 数据日志记录器 @param parent 父对象 */
RecordingController::RecordingController(DataLogger* logger, QObject* parent)
    : QObject(parent)
    , m_logger(logger)
{
    connect(m_logger, &DataLogger::playbackData,
            this, &RecordingController::onPlaybackData);
    connect(m_logger, &DataLogger::playbackProgress,
            this, &RecordingController::onPlaybackProgress);
    connect(m_logger, &DataLogger::recordingStopped,
            this, &RecordingController::onRecordingStopped);
    connect(m_logger, &DataLogger::recordingStarted, this, [this]() {
        ++m_totalRecordings;
        emit statusMessage(tr("录制已开始"), 2000);
    });
    connect(m_logger, &DataLogger::playbackFinished, this, [this]() {
        if (m_stopPlaybackAction) m_stopPlaybackAction->setEnabled(false);
        if (m_playbackAction) m_playbackAction->setEnabled(true);
        emit statusMessage(tr("回放完成"), 3000);
    });
    connect(m_logger, &DataLogger::error, this, [this](const QString& msg) {
        ++m_totalErrors;
        emit statusMessage(msg, 5000);
    });
    // 统计: 书签创建计数(信号被外部转发触发)
    connect(this, &RecordingController::addBookmarkRequested,
            this, [this]() { ++m_totalBookmarks; });
}

/** @brief 将录制/回放按钮添加到工具栏 */
void RecordingController::setupActions(QToolBar* toolbar)
{
    m_recordAction = toolbar->addAction(tr("录制"));
    m_recordAction->setCheckable(true);
    m_recordAction->setChecked(false);

    m_stopRecordAction = toolbar->addAction(tr("停止录制"));
    m_stopRecordAction->setEnabled(false);

    m_playbackAction = toolbar->addAction(tr("回放日志"));

    m_stopPlaybackAction = toolbar->addAction(tr("停止回放"));
    m_stopPlaybackAction->setEnabled(false);

    connect(m_recordAction, &QAction::toggled,
            this, &RecordingController::onToggleRecording);
    connect(m_stopRecordAction, &QAction::triggered,
            this, &RecordingController::onStopRecording);
    connect(m_playbackAction, &QAction::triggered,
            this, &RecordingController::onOpenPlayback);
    connect(m_stopPlaybackAction, &QAction::triggered,
            this, &RecordingController::onStopPlayback);
}

/** @brief 连接状态变化通知 */
void RecordingController::setConnected(bool connected)
{
    m_connected = connected;
}

// 统计getter和resetRecordingStatistics()见 RecordingControllerStats.cpp

/** @brief 录制按钮切换处理(未录制→开始 | 录制中→暂停 | 已暂停→继续) */
void RecordingController::onToggleRecording()
{
    if (!m_logger) {
        qWarning() << "RecordingController::onToggleRecording: m_logger is null";
        return;
    }
    if (m_logger->isRecording()) {
        if (m_logger->isPaused()) {
            m_logger->resumeRecording();
            m_recordAction->setText(tr("暂停"));
        } else {
            m_logger->pauseRecording();
            m_recordAction->setText(tr("继续"));
        }
    } else {
        QString filter = tr("EmbedDebug 日志 (*.edl);;所有文件 (*.*)");
        QString path = QFileDialog::getSaveFileName(
            qobject_cast<QWidget*>(parent()), tr("录制日志"), QString(), filter);
        if (path.isEmpty()) {
            m_recordAction->blockSignals(true);
            m_recordAction->setChecked(false);
            m_recordAction->blockSignals(false);
            return;
        }
        if (!path.endsWith(".edl")) path += ".edl";

        if (!m_logger->startRecording(path)) {
            m_recordAction->blockSignals(true);
            m_recordAction->setChecked(false);
            m_recordAction->blockSignals(false);
            ++m_totalErrors;
            ++m_totalRecordingErrors;
            emit statusMessage(tr("录制启动失败，请检查文件路径和权限"), 5000);
            return;
        }
        m_stopRecordAction->setEnabled(true);
        m_recordAction->setText(tr("暂停"));
        emit statusMessage(tr("录制中: %1").arg(path));
    }
}

/** @brief 停止录制按钮处理 */
void RecordingController::onStopRecording()
{
    if (!m_logger) return;
    m_logger->stopRecording();
    m_recordAction->setChecked(false);
    m_recordAction->setText(tr("录制"));
    m_stopRecordAction->setEnabled(false);
}

/** @brief 打开日志文件并开始回放 */
void RecordingController::onOpenPlayback()
{
    if (!m_logger) return;
    QString filter = tr("EmbedDebug 日志 (*.edl);;所有文件 (*.*)");
    QString path = QFileDialog::getOpenFileName(
        qobject_cast<QWidget*>(parent()), tr("打开日志回放"), QString(), filter);
    if (path.isEmpty()) return;

    if (!m_logger->startPlayback(path)) {
        ++m_totalErrors;
        emit statusMessage(tr("回放启动失败，请检查文件格式"), 5000);
        return;
    }
    ++m_totalPlaybacks;
    m_stopPlaybackAction->setEnabled(true);
    m_playbackAction->setEnabled(false);
    emit statusMessage(tr("回放中: %1").arg(path));
}

/** @brief 停止当前回放 */
void RecordingController::onStopPlayback()
{
    m_logger->stopPlayback();
    m_stopPlaybackAction->setEnabled(false);
    m_playbackAction->setEnabled(true);
}

/** @brief 回放数据转发 — 增加回放字节计数 */
void RecordingController::onPlaybackData(const QByteArray& data, qint64 direction)
{
    m_totalBytesPlayed += static_cast<quint64>(data.size());
    emit playbackData(data, direction);
}

/** @brief 回放进度更新 */
void RecordingController::onPlaybackProgress(qreal percent)
{
    emit statusMessage(tr("回放进度: %1%").arg(static_cast<int>(percent * 100)));
}

/** @brief 录制停止通知 — 更新时长/帧数统计 */
void RecordingController::onRecordingStopped(const QString& filePath, int count, qint64 durationMs)
{
    /* 统计：更新录制时长和帧数 */
    m_totalRecordedMs += durationMs;
    m_longestRecordingMs = qMax(m_longestRecordingMs, durationMs);
    m_totalFramesRecorded += static_cast<quint64>(count);

    emit statusMessage(
        tr("录制已保存: %1 (%2 条记录, %3秒)")
            .arg(filePath)
            .arg(count)
            .arg(durationMs / 1000.0, 0, 'f', 1),
        5000);
}
