#ifndef RECORDINGCONTROLLER_H
#define RECORDINGCONTROLLER_H

#include <QObject>
#include <QAction>

class QToolBar;
class DataLogger;

// 录制/回放控制器 - 从MainWindow中提取的录制回放业务逻辑
// 管理录制按钮、回放按钮的创建和交互，委托DataLogger完成实际操作
class RecordingController : public QObject {
    Q_OBJECT

public:
    explicit RecordingController(DataLogger* logger, QObject* parent = nullptr);

    // 将录制/回放按钮添加到工具栏（由MainWindow在setupToolbar时调用）
    void setupActions(QToolBar* toolbar);

    // 连接状态变化时启用/禁用录制按钮
    void setConnected(bool connected);

signals:
    // 状态栏消息通知（MainWindow用于显示到statusBar）
    void statusMessage(const QString& msg, int timeoutMs = 0);

    // 回放数据输出（MainWindow用于写入终端模型）
    void playbackData(const QByteArray& data, qint64 direction);

private slots:
    void onToggleRecording();
    void onStopRecording();
    void onOpenPlayback();
    void onStopPlayback();
    void onPlaybackData(const QByteArray& data, qint64 direction);
    void onPlaybackProgress(qreal percent);
    void onRecordingStopped(const QString& filePath, int count, qint64 duration);

private:
    DataLogger* m_logger;

    // 工具栏按钮
    QAction* m_recordAction = nullptr;
    QAction* m_stopRecordAction = nullptr;
    QAction* m_playbackAction = nullptr;
    QAction* m_stopPlaybackAction = nullptr;

    bool m_connected = false;
};

#endif // RECORDINGCONTROLLER_H
