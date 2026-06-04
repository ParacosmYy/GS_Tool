/**
 * @file RecordingController.h
 * @brief 录制控制器 - 管理数据录制和回放功能
 *
 * 职责: 录制/回放工具栏按钮 / 录制开始/暂停/继续/停止 / 回放打开/停止 /
 * DataLogger信号转发给MainWindow / 录制/回放累计统计
 */
#ifndef RECORDINGCONTROLLER_H
#define RECORDINGCONTROLLER_H

#include <QObject>
#include <QAction>

class QToolBar; class DataLogger;

/** @brief 录制/回放控制器 - 委托模式(DataLogger) + 观察者模式(Qt信号/槽)
 *  协作: DataLogger(底层录制/回放) / ConnectionController(连接状态) / MainWindow(UI) */
class RecordingController : public QObject {
    Q_OBJECT

public:
    explicit RecordingController(DataLogger* logger, QObject* parent = nullptr);
    void setupActions(QToolBar* toolbar);  ///< 将录制/回放按钮添加到工具栏
    void setConnected(bool connected);     ///< 连接状态变化通知，控制按钮启用/禁用

    quint64 totalRecordings() const;       ///< 获取累计录制次数
    quint64 totalPlaybacks() const;        ///< 获取累计回放次数
    quint64 totalBytesPlayed() const;      ///< 获取累计回放字节数
    quint64 totalErrors() const;           ///< 获取累计错误次数
    quint64 totalBookmarks() const;        ///< 获取累计书签创建数
    quint64 totalRecordingErrors() const;  ///< 获取累计录制错误数

    // ---- 录制时长与帧统计 ----
    qint64 totalRecordedMs() const;        ///< 获取累计录制总时长(ms)
    qint64 longestRecordingMs() const;     ///< 获取单次最长录制时长(ms)
    quint64 totalFramesRecorded() const;   ///< 获取累计录制数据帧总数

    // ---- 标记统计 ----
    quint64 markersCreated() const;        ///< 获取累计创建标记数
    quint64 markersNavigated() const;      ///< 获取累计导航标记数
    quint64 totalPauses() const;           ///< 获取累计暂停次数
    quint64 totalResumes() const;          ///< 获取累计恢复次数
    quint64 totalSegmentWrites() const;    ///< 获取累计段写入次数
    quint64 totalPlaybackStops() const;    ///< 获取累计回放停止次数
    void resetRecordingStatistics();

signals:
    void statusMessage(const QString& msg, int timeoutMs = 0); ///< 状态栏消息通知
    void playbackData(const QByteArray& data, qint64 direction); ///< 回放数据输出
    void addBookmarkRequested(const QString& label);          ///< 请求添加书签

private slots:
    void onToggleRecording();
    void onStopRecording();
    void onOpenPlayback();
    void onStopPlayback();
    void onPlaybackData(const QByteArray& data, qint64 direction);
    void onPlaybackProgress(qreal percent);
    void onRecordingStopped(const QString& filePath, int count, qint64 durationMs);

private:
    DataLogger* m_logger;
    QAction* m_recordAction = nullptr;
    QAction* m_stopRecordAction = nullptr;
    QAction* m_playbackAction = nullptr;
    QAction* m_stopPlaybackAction = nullptr;
    bool m_connected = false;

    quint64 m_totalRecordings = 0, m_totalPlaybacks = 0, m_totalBytesPlayed = 0;
    quint64 m_totalErrors = 0, m_totalBookmarks = 0, m_totalRecordingErrors = 0;
    qint64 m_totalRecordedMs = 0, m_longestRecordingMs = 0;
    quint64 m_totalFramesRecorded = 0;
    quint64 m_markersCreated = 0, m_markersNavigated = 0;
    quint64 m_totalPauses = 0, m_totalResumes = 0, m_totalSegmentWrites = 0, m_totalPlaybackStops = 0;
};

#endif // RECORDINGCONTROLLER_H
