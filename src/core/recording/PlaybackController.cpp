/**
 * @file PlaybackController.cpp
 * @brief 回放控制器实现 - 基于定时器驱动回放进度，使用QElapsedTimer精确计时
 *
 * 时间累积模型：m_currentTimeMs 为已累积的基准时间，
 * m_elapsed 测量自上次累积以来的真实经过时间，
 * 当前播放位置 = m_currentTimeMs + elapsed * m_speed。
 */

#include "core/recording/PlaybackController.h"
#include <QTimer>
#include <algorithm>

/* ============================================================
 * 构造 / 析构
 * ============================================================ */

/** @brief 构造回放控制器，初始化倍速和定时器(间隔固定16ms约60fps，倍速通过elapsed*m_speed计算而非改变定时器频率) @param parent 父对象指针 */
PlaybackController::PlaybackController(QObject* parent)
    : QObject(parent)
    , m_speed(1.0)
    , m_playing(false)
    , m_timer(new QTimer(this))
    , m_durationMs(0)
    , m_currentTimeMs(0)
{
    connect(m_timer, &QTimer::timeout,
            this,    &PlaybackController::onTick);
}

/** @brief 析构函数，确保定时器停止 */
PlaybackController::~PlaybackController()
{
    if (m_timer) {
        m_timer->stop();
    }
}

/* ============================================================
 * 播放控制
 * ============================================================ */

/** @brief 开始或继续回放，总时长未设置(<=0)或已在播放时忽略，启动精确计时器并以16ms间隔驱动 @sa pause @sa stop */
void PlaybackController::play()
{
    /* 无数据可播放 */
    if (m_durationMs <= 0) {
        return;
    }

    /* 已在播放，避免重复启动 elapsed */
    if (m_playing) {
        return;
    }

    m_playing = true;
    m_elapsed.start();
    m_timer->start(16);

    // 统计：累计回放启动计数
    ++m_totalPlaybacks;

    emit playbackStarted();
}

/** @brief 暂停回放，将自上次累积以来的经过时间(乘以倍速)累加到m_currentTimeMs，停止定时器保留当前进度以便后续续播 */
void PlaybackController::pause()
{
    if (!m_playing) {
        return;
    }

    m_playing = false;

    /* 累积已播放时间 */
    m_currentTimeMs += static_cast<qint64>(
        static_cast<qreal>(m_elapsed.elapsed()) * m_speed);

    /* 钳位：防止暂停点超出总时长 */
    if (m_currentTimeMs > m_durationMs) {
        m_currentTimeMs = m_durationMs;
    }

    m_timer->stop();

    // 统计：累计暂停计数
    ++m_totalPauses;

    emit playbackPaused();
}

/** @brief 停止回放并重置进度到起点 */
void PlaybackController::stop()
{
    m_playing = false;
    m_timer->stop();
    m_currentTimeMs = 0;

    // 统计：累计停止计数
    ++m_totalStops;

    emit playbackStopped();
}

/* ============================================================
 * 倍速 / 定位
 * ============================================================ */

/** @brief 设置回放倍速，若当前正在播放先累积到目前为止的时间再重启elapsed以避免速度切换导致的时间跳变 @param speed 目标倍速(1.0=正常速度) */
void PlaybackController::setSpeed(qreal speed)
{
    if (qFuzzyCompare(m_speed, speed)) {
        return;
    }

    /* 播放中切换倍速：先累积再重启计时器 */
    if (m_playing) {
        m_currentTimeMs += static_cast<qint64>(
            static_cast<qreal>(m_elapsed.elapsed()) * m_speed);
        m_elapsed.restart();
    }

    m_speed = speed;
    m_speedSum += speed;
    emit speedChanged(m_speed);
}

/** @brief 定位到指定时间点，自动钳位到[0,m_durationMs]范围，若当前正在播放则重启elapsed计时器 @param timeMs 目标时间位置(毫秒) */
void PlaybackController::seekTo(qint64 timeMs)
{
    /* 钳位到有效范围 */
    timeMs = std::clamp(timeMs, qint64(0), m_durationMs);

    m_currentTimeMs = timeMs;

    /* 播放中定位：重启精确计时器 */
    if (m_playing) {
        m_elapsed.restart();
    }

    // 统计：累计定位计数
    ++m_totalSeeks;

    emit timeUpdated(m_currentTimeMs);
}

/* ============================================================
 * 状态查询
 * ============================================================ */

/** @brief 获取当前回放倍速 @return 当前倍速值 */
qreal PlaybackController::speed() const
{
    return m_speed;
}

/** @brief 查询是否正在回放 @return true表示正在回放 */
bool PlaybackController::isPlaying() const
{
    return m_playing;
}

/** @brief 获取当前回放位置(毫秒)，播放中时包含实时估算m_currentTimeMs+elapsed*speed，非播放状态返回上次累积的基准时间 @return 当前时间位置(毫秒) */
qint64 PlaybackController::currentTimeMs() const
{
    if (m_playing) {
        return m_currentTimeMs + static_cast<qint64>(
            static_cast<qreal>(m_elapsed.elapsed()) * m_speed);
    }
    return m_currentTimeMs;
}

/** @brief 获取回放总时长 @return 总时长(毫秒) */
qint64 PlaybackController::durationMs() const
{
    return m_durationMs;
}

/** @brief 设置回放总时长，若当前未播放同时重置回放进度到起点 @param durationMs 总时长(毫秒) */
void PlaybackController::setDuration(qint64 durationMs)
{
    m_durationMs = durationMs;

    /* 未播放时重置进度 */
    if (!m_playing) {
        m_currentTimeMs = 0;
    }
}

/* ============================================================
 * 定时器驱动
 * ============================================================ */

/** @brief 定时器超时处理驱动回放进度，计算当前实时位置并判断是否到达终点：到达终点则停止回放发射playbackFinished，未到达则发射timeUpdated上报当前位置 */
void PlaybackController::onTick()
{
    if (!m_playing) {
        return;
    }

    const qint64 elapsed = m_currentTimeMs + static_cast<qint64>(
        static_cast<qreal>(m_elapsed.elapsed()) * m_speed);

    if (elapsed >= m_durationMs) {
        /* 到达终点：停止并通知 */
        m_playing = false;
        m_timer->stop();
        m_currentTimeMs = m_durationMs;
        ++m_playCount;
        m_totalPlayTimeMs += m_durationMs;

        emit playbackFinished();
        emit playbackStopped();
    } else {
        emit timeUpdated(elapsed);
    }
}

/** @brief 获取已完成回放次数 @return 回放完成计数 */
int PlaybackController::playCount() const
{
    return m_playCount;
}

/** @brief 获取累计回放时长 @return 累计播放时间(毫秒) */
qint64 PlaybackController::totalPlayTimeMs() const
{
    return m_totalPlayTimeMs;
}

/** @brief 获取平均回放倍速 @return 平均倍速值，无回放记录时返回0.0 */
qreal PlaybackController::averageSpeed() const
{
    if (m_playCount == 0) {
        return 0.0;
    }
    return m_speedSum / static_cast<qreal>(m_playCount);
}

/** @brief 重置基础统计数据(回放次数/累计时长/倍速总和) */
void PlaybackController::resetStatistics()
{
    m_playCount = 0;
    m_totalPlayTimeMs = 0;
    m_speedSum = 0.0;
}

/* ============================================================
 * 统计接口
 * ============================================================ */

/** @brief 获取累计回放启动次数 @return 启动总次数 */
quint64 PlaybackController::totalPlaybacks() const
{
    return m_totalPlaybacks;
}

/** @brief 获取累计暂停次数 @return 暂停总次数 */
quint64 PlaybackController::totalPauses() const
{
    return m_totalPauses;
}

/** @brief 获取累计停止次数 @return 停止总次数 */
quint64 PlaybackController::totalStops() const
{
    return m_totalStops;
}

/** @brief 获取累计定位次数 @return 定位总次数 */
quint64 PlaybackController::totalSeeks() const
{
    return m_totalSeeks;
}

/** @brief 获取平均回放倍速 @return 平均倍速值，无回放记录时返回0.0 */
qreal PlaybackController::averagePlaybackSpeed() const
{
    return averageSpeed();
}

/** @brief 获取累计回放总时长(毫秒) @return 累计播放时长 */
qint64 PlaybackController::totalPlaybackDurationMs() const
{
    return m_totalPlayTimeMs;
}

/** @brief 重置所有统计计数器(含基础统计和扩展统计) */
void PlaybackController::resetStats()
{
    // 基础统计
    m_playCount = 0;
    m_totalPlayTimeMs = 0;
    m_speedSum = 0.0;
    // 扩展统计
    m_totalPlaybacks = 0;
    m_totalPauses = 0;
    m_totalStops = 0;
    m_totalSeeks = 0;
}
