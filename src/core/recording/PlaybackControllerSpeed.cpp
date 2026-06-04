/**
 * @file PlaybackControllerSpeed.cpp
 * @brief 回放控制器 - 倍速控制/定位/状态查询/统计接口实现
 *
 * 本文件从 PlaybackController.cpp 拆分而来，包含：
 * - 倍速设置与查询
 * - 时间定位(seekTo)
 * - 状态查询(当前位置/总时长)
 * - 基础统计(回放次数/累计时长/平均倍速)
 * - 扩展统计(启动/暂停/停止/定位/倍速变更计数)
 * - 统计重置
 */

#include "core/recording/PlaybackController.h"
#include <algorithm>

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
    ++m_totalSpeedChanges;
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
 * 基础统计
 * ============================================================ */

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
 * 扩展统计接口
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

/** @brief 获取累计倍速变更次数 @return 倍速变更总次数 */
quint64 PlaybackController::totalSpeedChanges() const
{
    return m_totalSpeedChanges;
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
    m_totalSpeedChanges = 0;
}
