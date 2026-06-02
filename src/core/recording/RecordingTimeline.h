/**
 * @file RecordingTimeline.h
 * @brief 录制时间线管理器，负责数据录制过程中的时间轴控制
 *
 * 提供录制启停、时间追踪、事件时间戳记录等功能，
 * 为数据录制回放子系统(F1)提供时间基准。
 */

#ifndef RECORDING_TIMELINE_H
#define RECORDING_TIMELINE_H

#include <QObject>
#include <QList>

/**
 * @class RecordingTimeline
 * @brief 录制时间线管理类
 *
 * 管理录制会话的生命周期，维护绝对时间和相对时间，
 * 记录关键事件的时间戳用于后续回放定位。
 */
class RecordingTimeline : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父对象指针
     */
    explicit RecordingTimeline(QObject* parent = nullptr);

    /**
     * @brief 开始录制
     *
     * 重置时间基准并进入录制状态，发出 recordingStarted() 信号。
     */
    void startRecording();

    /**
     * @brief 停止录制
     *
     * 结束当前录制会话，发出 recordingStopped() 信号。
     */
    void stopRecording();

    /**
     * @brief 获取当前已录制时长
     * @return 从录制开始到当前的毫秒数，未录制时返回上次录制时长
     */
    qint64 currentTimeMs() const;

    /**
     * @brief 定位到指定时间点
     * @param timeMs 目标时间点（毫秒）
     */
    void seekTo(qint64 timeMs);

    /**
     * @brief 获取所有事件时间戳
     * @return 事件时间戳列表（毫秒），按升序排列
     */
    QList<qint64> eventTimestamps() const;

    /**
     * @brief 记录一个事件时间戳
     *
     * 将指定时间戳添加到事件列表中，用于标记录制过程中的关键时间点。
     * 时间戳必须在有效范围内 [0, 当前已录制时长]。
     *
     * @param timestampMs 事件发生的时间点（毫秒）
     */
    void recordEvent(qint64 timestampMs);

    /**
     * @brief 清除所有已记录的事件
     */
    void clearEvents();

    /**
     * @brief 获取录制总时长
     * @return 上次录制结束时的总时长（毫秒），录制中返回当前已过时长
     */
    qint64 totalDurationMs() const;

signals:
    /**
     * @brief 时间更新信号
     * @param timeMs 当前时间（毫秒）
     */
    void timeUpdated(qint64 timeMs);

    /**
     * @brief 录制已启动信号
     */
    void recordingStarted();

    /**
     * @brief 录制已停止信号
     * @param totalMs 本次录制的总时长（毫秒）
     */
    void recordingStopped(qint64 totalMs);

private:
    qint64 m_startTimeMs;   ///< 录制起始的绝对时间（毫秒）
    qint64 m_elapsedMs;     ///< 已录制的累计时长（毫秒，仅stopRecording更新）
    qint64 m_seekPositionMs;///< 回放定位位置（毫秒，仅seekTo更新）
    bool   m_recording;     ///< 当前是否正在录制
    QList<qint64> m_events; ///< 已记录的事件时间戳列表（毫秒）
};

#endif // RECORDING_TIMELINE_H
