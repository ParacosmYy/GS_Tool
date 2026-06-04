/** @file PlaybackController.h @brief 回放控制器，管理录制数据的播放/暂停/变速/定位。支持变速播放和随机定位，为F1数据录制回放子系统的回放核心 */

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
    explicit PlaybackController(QObject* parent = nullptr); ///< 构造函数
    ~PlaybackController() override;                         ///< 析构函数
    void play();                              ///< 开始或继续回放
    void pause();                             ///< 暂停回放
    void stop();                              ///< 停止回放并重置进度
    void setSpeed(qreal speed);               ///< 设置回放倍速(1.0=正常)
    void seekTo(qint64 timeMs);               ///< 定位到指定时间点(ms)
    qreal speed() const;                      ///< 获取当前回放倍速
    bool isPlaying() const;                   ///< 查询是否正在回放
    void setDuration(qint64 durationMs);      ///< 设置回放总时长(ms)
    qint64 currentTimeMs() const;             ///< 获取当前回放位置(ms)
    qint64 durationMs() const;                ///< 获取回放总时长(ms)
    int playCount() const;                    ///< 已完成回放次数
    qint64 totalPlayTimeMs() const;           ///< 累计回放时长(ms)
    qreal averageSpeed() const;               ///< 平均回放倍速
    void resetStatistics();                   ///< 重置统计数据

    // ==================== 统计接口 ====================
    quint64 totalPlaybacks() const;           ///< 累计回放启动次数
    quint64 totalPauses() const;              ///< 累计暂停次数
    quint64 totalStops() const;               ///< 累计停止次数
    quint64 totalSeeks() const;               ///< 累计定位次数
    quint64 totalSpeedChanges() const;        ///< 累计倍速变更次数
    quint64 totalPlaybackResumes() const;     ///< 累计恢复播放(从暂停恢复)次数
    qreal averagePlaybackSpeed() const;       ///< 平均回放倍速(同averageSpeed)
    qint64 totalPlaybackDurationMs() const;   ///< 累计回放总时长(ms, 同totalPlayTimeMs)
    void resetStats();                        ///< 重置所有统计计数器

signals:
    void playbackStarted();                   ///< 回放已启动信号
    void playbackPaused();                    ///< 回放已暂停信号
    void playbackStopped();                   ///< 回放已停止信号
    void speedChanged(qreal speed);           ///< 倍速变更信号
    void timeUpdated(qint64 timeMs);          ///< 回放时间更新信号(~60fps)
    void playbackFinished();                  ///< 回放到达终点信号

private:
    void onTick();                            ///< 定时器超时处理，驱动回放进度

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
};

#endif // PLAYBACK_CONTROLLER_H
