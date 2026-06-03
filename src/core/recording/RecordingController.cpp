/**
 * @file RecordingController.cpp
 * @brief 录制/回放控制器实现 - 管理录制/回放按钮的交互和数据流转发
 */

#include "core/recording/RecordingController.h"
#include "utils/log/DataLogger.h"
#include <QToolBar>
#include <QFileDialog>

/**
 * @brief 构造录制控制器
 * 连接 DataLogger 的信号到内部槽，建立录制/回放数据流管道
 * @param logger 数据日志记录器，底层的录制/回放引擎
 * @param parent 父对象
 */
RecordingController::RecordingController(DataLogger* logger, QObject* parent)
    : QObject(parent)
    , m_logger(logger)
{
    // DataLogger 信号 → RecordingController 槽
    connect(m_logger, &DataLogger::playbackData,
            this, &RecordingController::onPlaybackData);
    connect(m_logger, &DataLogger::playbackProgress,
            this, &RecordingController::onPlaybackProgress);
    connect(m_logger, &DataLogger::recordingStopped,
            this, &RecordingController::onRecordingStopped);
    // 录制开始: 显示状态提示
    connect(m_logger, &DataLogger::recordingStarted, this, [this]() {
        emit statusMessage(tr("录制已开始"), 2000);
    });
    // 回放完成: 恢复按钮状态
    connect(m_logger, &DataLogger::playbackFinished, this, [this]() {
        if (m_stopPlaybackAction) m_stopPlaybackAction->setEnabled(false);
        if (m_playbackAction) m_playbackAction->setEnabled(true);
        emit statusMessage(tr("回放完成"), 3000);
    });
    // 错误通知
    connect(m_logger, &DataLogger::error, this, [this](const QString& msg) {
        emit statusMessage(msg, 5000);
    });
}

/**
 * @brief 将录制/回放按钮添加到工具栏
 * 创建四个按钮: 录制(可切换) | 停止录制 | 回放日志 | 停止回放
 * @param toolbar 主窗口的工具栏
 */
void RecordingController::setupActions(QToolBar* toolbar)
{
    // 日志录制按钮（可切换: 未录制→开始录制, 录制中→暂停/继续）
    m_recordAction = toolbar->addAction(tr("录制"));
    m_recordAction->setCheckable(true);
    m_recordAction->setChecked(false);

    // 停止录制按钮（仅在录制进行中启用）
    m_stopRecordAction = toolbar->addAction(tr("停止录制"));
    m_stopRecordAction->setEnabled(false);

    // 日志回放按钮（打开文件对话框选择 .edl 日志文件）
    m_playbackAction = toolbar->addAction(tr("回放日志"));

    // 停止回放按钮（仅在回放进行中启用）
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

/**
 * @brief 连接状态变化通知
 * @param connected true=已连接, false=已断开
 */
void RecordingController::setConnected(bool connected)
{
    m_connected = connected;
}

/**
 * @brief 录制按钮切换处理
 *
 * 三种状态转换:
 *   未录制 + 点击 → 开始录制（弹出文件对话框）
 *   录制中 + 点击 → 暂停录制
 *   已暂停 + 点击 → 继续录制
 */
void RecordingController::onToggleRecording()
{
    if (!m_logger) {
        qWarning() << "RecordingController::onToggleRecording: m_logger is null";
        return;
    }
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
        // 开始录制: 弹出文件对话框选择保存路径
        QString filter = tr("EmbedDebug 日志 (*.edl);;所有文件 (*.*)");
        QString path = QFileDialog::getSaveFileName(
            qobject_cast<QWidget*>(parent()), tr("录制日志"), QString(), filter);
        if (path.isEmpty()) {
            // 用户取消，恢复按钮状态（阻塞信号避免触发递归 onToggleRecording）
            m_recordAction->blockSignals(true);
            m_recordAction->setChecked(false);
            m_recordAction->blockSignals(false);
            return;
        }
        if (!path.endsWith(".edl")) path += ".edl";

        if (!m_logger->startRecording(path)) {
            // 录制启动失败(文件权限/磁盘满)，恢复按钮状态
            m_recordAction->blockSignals(true);
            m_recordAction->setChecked(false);
            m_recordAction->blockSignals(false);
            emit statusMessage(tr("录制启动失败，请检查文件路径和权限"), 5000);
            return;
        }
        m_stopRecordAction->setEnabled(true);
        m_recordAction->setText(tr("暂停"));
        emit statusMessage(tr("录制中: %1").arg(path));
    }
}

/**
 * @brief 停止录制按钮处理
 * 停止录制并恢复所有按钮到初始状态
 */
void RecordingController::onStopRecording()
{
    if (!m_logger) return;
    m_logger->stopRecording();
    m_recordAction->setChecked(false);
    m_recordAction->setText(tr("录制"));
    m_stopRecordAction->setEnabled(false);
}

/**
 * @brief 打开日志文件并开始回放
 * 弹出文件对话框选择 .edl 日志文件，开始回放
 */
void RecordingController::onOpenPlayback()
{
    if (!m_logger) return;
    QString filter = tr("EmbedDebug 日志 (*.edl);;所有文件 (*.*)");
    QString path = QFileDialog::getOpenFileName(
        qobject_cast<QWidget*>(parent()), tr("打开日志回放"), QString(), filter);
    if (path.isEmpty()) return;

    if (!m_logger->startPlayback(path)) {
        emit statusMessage(tr("回放启动失败，请检查文件格式"), 5000);
        return;
    }
    m_stopPlaybackAction->setEnabled(true);
    m_playbackAction->setEnabled(false);
    emit statusMessage(tr("回放中: %1").arg(path));
}

/**
 * @brief 停止当前回放
 * 停止回放并恢复按钮状态
 */
void RecordingController::onStopPlayback()
{
    m_logger->stopPlayback();
    m_stopPlaybackAction->setEnabled(false);
    m_playbackAction->setEnabled(true);
}

/**
 * @brief 回放数据转发
 * 将 DataLogger 的回放数据转发给 MainWindow 写入终端
 * @param data 回放的字节数据
 * @param direction 数据方向: 0=接收(绿色), 1=发送(蓝色)
 */
void RecordingController::onPlaybackData(const QByteArray& data, qint64 direction)
{
    emit playbackData(data, direction);
}

/**
 * @brief 回放进度更新
 * @param percent 回放进度百分比 (0.0~1.0)
 */
void RecordingController::onPlaybackProgress(qreal percent)
{
    emit statusMessage(tr("回放进度: %1%").arg(static_cast<int>(percent * 100)));
}

/**
 * @brief 录制停止通知
 * DataLogger 在录制停止时发出此信号，包含录制统计信息
 * @param filePath 录制文件路径
 * @param count 录制的记录数量
 * @param durationMs 录制总时长（毫秒）
 */
void RecordingController::onRecordingStopped(const QString& filePath, int count, qint64 durationMs)
{
    emit statusMessage(
        tr("录制已保存: %1 (%2 条记录, %3秒)")
            .arg(filePath)
            .arg(count)
            .arg(durationMs / 1000.0, 0, 'f', 1),
        5000);
}
