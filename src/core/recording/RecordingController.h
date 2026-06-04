/**
 * @file RecordingController.h
 * @brief 录制控制器 - 管理数据录制和回放功能
 *
 * 职责:
 *   1. 创建和管理录制/回放工具栏按钮
 *   2. 处理录制开始/暂停/继续/停止的交互逻辑
 *   3. 处理回放打开/停止的交互逻辑
 *   4. 将 DataLogger 的信号转发给 MainWindow 用于 UI 更新
 *   5. 跟踪录制/回放的累计统计数据
 */

#ifndef RECORDINGCONTROLLER_H
#define RECORDINGCONTROLLER_H

#include <QObject>
#include <QAction>

class QToolBar;
class DataLogger;

/**
 * @brief 录制/回放控制器 - 管理数据录制和回放的完整交互流程
 *
 * 设计模式:
 *   - 委托模式: 将实际录制/回放操作委托给 DataLogger
 *   - 观察者模式: 通过 Qt 信号/槽通知状态变化
 *
 * 协作关系:
 *   - DataLogger: 底层录制/回放引擎，完成文件 I/O 和时间戳管理
 *   - ConnectionController: 连接状态变化时通知此控制器启用/禁用录制按钮
 *   - MainWindow: 接收回放数据和状态消息，更新终端和状态栏
 */
class RecordingController : public QObject {
    Q_OBJECT

public:
    /**
     * @brief 构造录制控制器
     * @param logger 数据日志记录器
     * @param parent 父对象
     */
    explicit RecordingController(DataLogger* logger, QObject* parent = nullptr);

    /** @brief 将录制/回放按钮添加到工具栏 */
    void setupActions(QToolBar* toolbar);

    /** @brief 连接状态变化通知，控制录制按钮的启用/禁用 */
    void setConnected(bool connected);

    /** @brief 获取累计录制次数 */
    quint64 totalRecordings() const;
    /** @brief 获取累计回放次数 */
    quint64 totalPlaybacks() const;
    /** @brief 获取累计回放字节数 */
    quint64 totalBytesPlayed() const;
    /** @brief 获取累计错误次数（录制/回放启动失败等） */
    quint64 totalErrors() const;
    /** @brief 获取累计书签创建数 */
    quint64 totalBookmarks() const;
    /** @brief 获取累计录制错误数(仅录制启动/写入失败) */
    quint64 totalRecordingErrors() const;

    // ==================== 录制时长与帧统计 ====================

    /** @brief 获取累计录制总时长(毫秒) @return 时长总和 */
    qint64 totalRecordedMs() const;

    /** @brief 获取单次最长录制时长(毫秒) @return 最长时长 */
    qint64 longestRecordingMs() const;

    /** @brief 获取累计录制数据帧总数 @return 帧总数 */
    quint64 totalFramesRecorded() const;

    // ==================== 标记统计 ====================

    /** @brief 获取累计创建的标记总数 @return 标记创建数 */
    quint64 markersCreated() const;

    /** @brief 获取累计导航(跳转)的标记总数 @return 标记导航数 */
    quint64 markersNavigated() const;

    /** @brief 获取累计录制暂停次数 @return 暂停次数 */
    quint64 totalPauses() const;

    /** @brief 获取累计录制恢复次数 @return 恢复次数 */
    quint64 totalResumes() const;

    /** @brief 获取累计数据段写入次数 @return 段写入次数 */
    quint64 totalSegmentWrites() const;

    /** @brief 获取累计回放手动停止次数 @return 回放停止次数 */
    quint64 totalPlaybackStops() const;

    /** @brief 重置所有统计计数器 */
    void resetRecordingStatistics();

signals:
    /** @brief 状态栏消息通知 @param msg 消息文本 @param timeoutMs 显示时长(毫秒，0=默认) */
    void statusMessage(const QString& msg, int timeoutMs = 0);

    /** @brief 回放数据输出信号 @param data 回放的字节数据 @param direction 方向标识 */
    void playbackData(const QByteArray& data, qint64 direction);

    /** @brief 请求添加书签信号 @param label 书签标签文本 */
    void addBookmarkRequested(const QString& label);

private slots:
    /** @brief 录制按钮切换处理: 开始/暂停/继续录制 */
    void onToggleRecording();

    /** @brief 停止录制按钮处理 */
    void onStopRecording();

    /** @brief 打开日志文件并开始回放 */
    void onOpenPlayback();

    /** @brief 停止当前回放 */
    void onStopPlayback();

    /** @brief 回放数据转发 @param data 回放数据 @param direction 方向标识 */
    void onPlaybackData(const QByteArray& data, qint64 direction);

    /** @brief 回放进度更新 @param percent 完成百分比(0.0~1.0) */
    void onPlaybackProgress(qreal percent);

    /** @brief 录制停止通知 @param filePath 日志文件路径 @param count 录制的数据条数 @param durationMs 录制时长(毫秒) */
    void onRecordingStopped(const QString& filePath, int count, qint64 durationMs);

private:
    DataLogger* m_logger;                  ///< 数据日志记录器（底层录制/回放引擎）

    // ---- 工具栏按钮 ----
    QAction* m_recordAction = nullptr;     ///< 录制按钮（可切换）
    QAction* m_stopRecordAction = nullptr; ///< 停止录制按钮
    QAction* m_playbackAction = nullptr;   ///< 回放日志按钮
    QAction* m_stopPlaybackAction = nullptr; ///< 停止回放按钮
    bool m_connected = false;              ///< 当前是否已连接

    // ---- 统计计数器 ----
    quint64 m_totalRecordings = 0;         ///< 累计录制次数
    quint64 m_totalPlaybacks = 0;          ///< 累计回放次数
    quint64 m_totalBytesPlayed = 0;        ///< 累计回放字节数
    quint64 m_totalErrors = 0;             ///< 累计错误次数
    quint64 m_totalBookmarks = 0;          ///< 累计书签创建数
    quint64 m_totalRecordingErrors = 0;    ///< 累计录制错误数

    // ---- 录制时长与帧统计计数器 ----
    qint64 m_totalRecordedMs = 0;          ///< 累计录制总时长(毫秒)
    qint64 m_longestRecordingMs = 0;       ///< 单次最长录制时长(毫秒)
    quint64 m_totalFramesRecorded = 0;     ///< 累计录制数据帧总数

    // ---- 标记统计计数器 ----
    quint64 m_markersCreated = 0;          ///< 累计创建的标记总数
    quint64 m_markersNavigated = 0;        ///< 累计导航(跳转)的标记总数

    // ---- 新增统计计数器 ----
    quint64 m_totalPauses = 0;             ///< 累计录制暂停次数
    quint64 m_totalResumes = 0;            ///< 累计录制恢复次数
    quint64 m_totalSegmentWrites = 0;      ///< 累计数据段写入次数
    quint64 m_totalPlaybackStops = 0;      ///< 累计回放手动停止次数
};

#endif // RECORDINGCONTROLLER_H
