/**
 * @file RecordingController.h
 * @brief 录制控制器 - 管理数据录制和回放功能
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
 * 职责:
 *   1. 创建和管理录制/回放工具栏按钮
 *   2. 处理录制开始/暂停/继续/停止的交互逻辑
 *   3. 处理回放打开/停止的交互逻辑
 *   4. 将 DataLogger 的信号转发给 MainWindow 用于 UI 更新
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
     * 连接 DataLogger 的信号到内部槽，建立数据流管道
     * @param logger 数据日志记录器
     * @param parent 父对象
     */
    explicit RecordingController(DataLogger* logger, QObject* parent = nullptr);

    /**
     * @brief 将录制/回放按钮添加到工具栏
     * 创建: 录制(可切换) + 停止录制 + 回放日志 + 停止回放 四个按钮
     * @param toolbar 主窗口的工具栏
     */
    void setupActions(QToolBar* toolbar);

    /**
     * @brief 连接状态变化通知
     * 由 ConnectionController 调用，控制录制按钮的启用/禁用状态
     * @param connected true=已连接, false=已断开
     */
    void setConnected(bool connected);

signals:
    /**
     * @brief 状态栏消息通知
     * @param msg 消息文本
     * @param timeoutMs 消息显示时长（毫秒），0=默认
     */
    void statusMessage(const QString& msg, int timeoutMs = 0);

    /**
     * @brief 回放数据输出信号
     * @param data 回放的字节数据
     * @param direction 数据方向: 0=接收, 1=发送
     */
    void playbackData(const QByteArray& data, qint64 direction);

    /**
     * @brief 请求添加书签信号
     * 由 UI 交互触发（如工具栏书签按钮或快捷键），
     * MainWindow 应将此信号连接到 DataLogger::addBookmark
     * @param label 书签标签文本
     */
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

    /**
     * @brief 回放数据转发
     * 将 DataLogger 的回放数据转发给 MainWindow
     * @param data 回放的字节数据
     * @param direction 数据方向: 0=接收, 1=发送
     */
    void onPlaybackData(const QByteArray& data, qint64 direction);

    /**
     * @brief 回放进度更新
     * @param percent 回放进度百分比 (0.0~1.0)
     */
    void onPlaybackProgress(qreal percent);

    /**
     * @brief 录制停止通知
     * @param filePath 录制文件路径
     * @param count 录制的记录数量
     * @param durationMs 录制总时长（毫秒）
     */
    void onRecordingStopped(const QString& filePath, int count, qint64 durationMs);

private:
    /** @brief 数据日志记录器，底层的录制/回放引擎 */
    DataLogger* m_logger;

    // ---- 工具栏按钮 ----

    /** @brief 录制按钮（可切换: 未录制→开始录制, 录制中→暂停/继续） */
    QAction* m_recordAction = nullptr;

    /** @brief 停止录制按钮（仅在录制进行中启用） */
    QAction* m_stopRecordAction = nullptr;

    /** @brief 回放日志按钮（打开文件对话框选择 .edl 日志文件） */
    QAction* m_playbackAction = nullptr;

    /** @brief 停止回放按钮（仅在回放进行中启用） */
    QAction* m_stopPlaybackAction = nullptr;

    /** @brief 当前是否已连接（用于控制录制按钮的启用状态） */
    bool m_connected = false;
};

#endif // RECORDINGCONTROLLER_H
