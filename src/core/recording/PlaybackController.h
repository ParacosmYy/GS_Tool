/**
 * @file PlaybackController.h
 * @brief 回放控制器，管理录制数据的播放/暂停/变速/定位。支持变速播放和随机定位，为F1数据录制回放子系统的回放核心
 */

#ifndef PLAYBACK_CONTROLLER_H
#define PLAYBACK_CONTROLLER_H

#include <QObject>
#include <QElapsedTimer>

class QTimer;

/** @brief 回放控制器类。通过内部定时器驱动回放进度，支持播放/暂停/停止/变速/定位，回放进度通过timeUpdated信号上报 */
class PlaybackController : public QObject
{
    Q_OBJECT

public:
    /** @brief 构造函数 @param parent 父对象 */
    explicit PlaybackController(QObject* parent = nullptr);

    /** @brief 析构函数 */
    ~PlaybackController() override;

    /** @brief 开始或继续回放 */
    void play();

    /** @brief 暂停回放 */
    void pause();

    /** @brief 停止回放并重置进度 */
    void stop();

    /** @brief 设置回放倍速 @param speed 倍速值(1.0=正常速度) */
    void setSpeed(qreal speed);

    /** @brief 定位到指定时间点 @param timeMs 目标时间(毫秒) */
    void seekTo(qint64 timeMs);

    /** @brief 获取当前回放倍速 @return 倍速值 */
    qreal speed() const;

    /** @brief 查询是否正在回放 @return true=正在播放 */
    bool isPlaying() const;

    /** @brief 设置回放总时长 @param durationMs 总时长(毫秒) */
    void setDuration(qint64 durationMs);

    /** @brief 获取当前回放位置 @return 当前时间(毫秒) */
    qint64 currentTimeMs() const;

    /** @brief 获取回放总时长 @return 总时长(毫秒) */
    qint64 durationMs() const;

    /** @brief 获取已完成回放次数 @return 完成次数 */
    int playCount() const;

    /** @brief 获取累计回放时长 @return 累计毫秒数 */
    qint64 totalPlayTimeMs() const;

    /** @brief 获取平均回放倍速 @return 平均倍速值 */
    qreal averageSpeed() const;

    /** @brief 重置基础统计数据 */
    void resetStatistics();

    // ==================== 统计接口 ====================

    /** @brief 获取累计回放启动次数 @return 启动次数 */
    quint64 totalPlaybacks() const;

    /** @brief 获取累计暂停次数 @return 暂停次数 */
    quint64 totalPauses() const;

    /** @brief 获取累计停止次数 @return 停止次数 */
    quint64 totalStops() const;

    /** @brief 获取累计定位次数 @return 定位次数 */
    quint64 totalSeeks() const;

    /** @brief 获取累计倍速变更次数 @return 变更次数 */
    quint64 totalSpeedChanges() const;

    /** @brief 获取累计恢复播放(从暂停恢复)次数 @return 恢复次数 */
    quint64 totalPlaybackResumes() const;

    /** @brief 获取平均回放倍速(同averageSpeed) @return 平均倍速值 */
    qreal averagePlaybackSpeed() const;

    /** @brief 获取累计回放总时长(同totalPlayTimeMs) @return 累计毫秒数 */
    qint64 totalPlaybackDurationMs() const;

    /** @brief 获取累计回放完成(到达终点)次数 @return 完成次数 */
    quint64 totalCompletions() const;

    /** @brief 获取累计回放总时长变更次数 @return 变更次数 */
    quint64 totalDurationChanges() const;

    /** @brief 重置所有统计计数器 */
    void resetStats();

signals:
    /** @brief 回放已启动信号 */
    void playbackStarted();

    /** @brief 回放已暂停信号 */
    void playbackPaused();

    /** @brief 回放已停止信号 */
    void playbackStopped();

    /** @brief 倍速变更信号 @param speed 新倍速值 */
    void speedChanged(qreal speed);

    /** @brief 回放时间更新信号(~60fps) @param timeMs 当前时间(毫秒) */
    void timeUpdated(qint64 timeMs);

    /** @brief 回放到达终点信号 */
    void playbackFinished();

private:
    /** @brief 定时器超时处理，驱动回放进度 */
    void onTick();

    qreal m_speed;              ///< 当前回放倍速
    bool m_playing;             ///< 是否正在播放
    QTimer* m_timer;            ///< 驱动回放进度的定时器
    qint64 m_durationMs = 0;   ///< 回放总时长(ms)
    qint64 m_currentTimeMs = 0; ///< 当前已累积的回放位置(ms)
    QElapsedTimer m_elapsed;    ///< 精确计时器

    int m_playCount = 0;        ///< 已完成回放次数
    qint64 m_totalPlayTimeMs = 0; ///< 累计回放时长(ms)
    qreal m_speedSum = 0.0;     ///< 倍速累计(用于计算平均倍速)

    // ---- 统计计数器 ----
    quint64 m_totalPlaybacks = 0;       ///< 累计回放启动次数
    quint64 m_totalPauses = 0;          ///< 累计暂停次数
    quint64 m_totalStops = 0;           ///< 累计停止次数
    quint64 m_totalSeeks = 0;           ///< 累计定位次数
    quint64 m_totalSpeedChanges = 0;    ///< 累计倍速变更次数
    quint64 m_totalPlaybackResumes = 0; ///< 累计恢复播放次数
    quint64 m_totalCompletions = 0;    ///< 累计回放完成(到达终点)次数
    quint64 m_totalDurationChanges = 0; ///< 累计回放总时长变更次数
};

#endif // PLAYBACK_CONTROLLER_H
