/**
 * @file PlaybackController.cpp
 * @brief 回放控制器实现
 *
 * 基于定时器驱动回放进度，使用 QElapsedTimer 精确计时，
 * 支持变速播放、暂停续播、随机定位等操作。
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

/**
 * @brief 构造函数，初始化倍速和定时器
 * @param parent 父对象指针
 *
 * 定时器间隔固定 16ms（约 60fps），与倍速解耦。
 * 倍速效果通过 elapsed * m_speed 计算，而非改变定时器频率。
 */
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

/**
 * @brief 析构函数，确保定时器停止
 */
PlaybackController::~PlaybackController()
{
    if (m_timer) {
        m_timer->stop();
    }
}

/* ============================================================
 * 播放控制
 * ============================================================ */

/**
 * @brief 开始或继续回放
 *
 * 若总时长未设置（≤ 0）则忽略；若已在播放则跳过。
 * 启动精确计时器并以 16ms 间隔驱动定时器。
 */
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

    emit playbackStarted();
}

/**
 * @brief 暂停回放
 *
 * 将自上次累积以来的经过时间（乘以倍速）累加到 m_currentTimeMs，
 * 停止定时器，保留当前进度以便后续续播。
 */
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
    emit playbackPaused();
}

/**
 * @brief 停止回放并重置进度到起点
 */
void PlaybackController::stop()
{
    m_playing = false;
    m_timer->stop();
    m_currentTimeMs = 0;

    emit playbackStopped();
}

/* ============================================================
 * 倍速 / 定位
 * ============================================================ */

/**
 * @brief 设置回放倍速
 * @param speed 目标倍速（1.0 = 正常速度）
 *
 * 若当前正在播放，先累积到目前为止的时间，
 * 再重启 elapsed 以避免速度切换导致的时间跳变。
 */
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

/**
 * @brief 定位到指定时间点
 * @param timeMs 目标时间位置（毫秒）
 *
 * 自动钳位到 [0, m_durationMs] 范围内。
 * 若当前正在播放，重启 elapsed 计时器。
 */
void PlaybackController::seekTo(qint64 timeMs)
{
    /* 钳位到有效范围 */
    timeMs = std::clamp(timeMs, qint64(0), m_durationMs);

    m_currentTimeMs = timeMs;

    /* 播放中定位：重启精确计时器 */
    if (m_playing) {
        m_elapsed.restart();
    }

    emit timeUpdated(m_currentTimeMs);
}

/* ============================================================
 * 状态查询
 * ============================================================ */

/**
 * @brief 获取当前回放倍速
 * @return 当前倍速值
 */
qreal PlaybackController::speed() const
{
    return m_speed;
}

/**
 * @brief 查询是否正在回放
 * @return true 表示正在回放
 */
bool PlaybackController::isPlaying() const
{
    return m_playing;
}

/**
 * @brief 获取当前回放位置
 * @return 当前时间位置（毫秒）
 *
 * 播放中时包含实时估算：m_currentTimeMs + elapsed * speed。
 * 非播放状态返回上次累积的基准时间。
 */
qint64 PlaybackController::currentTimeMs() const
{
    if (m_playing) {
        return m_currentTimeMs + static_cast<qint64>(
            static_cast<qreal>(m_elapsed.elapsed()) * m_speed);
    }
    return m_currentTimeMs;
}

/**
 * @brief 获取回放总时长
 * @return 总时长（毫秒）
 */
qint64 PlaybackController::durationMs() const
{
    return m_durationMs;
}

/**
 * @brief 设置回放总时长
 * @param durationMs 总时长（毫秒）
 *
 * 若当前未播放，同时重置回放进度到起点。
 */
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

/**
 * @brief 定时器超时处理，驱动回放进度
 *
 * 计算当前实时位置并判断是否到达终点：
 * - 到达终点：停止回放，发射 playbackFinished
 * - 未到达：发射 timeUpdated 上报当前位置
 */
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

/**
 * @brief 获取已完成回放次数
 */
int PlaybackController::playCount() const
{
    return m_playCount;
}

/**
 * @brief 获取累计回放时长
 */
qint64 PlaybackController::totalPlayTimeMs() const
{
    return m_totalPlayTimeMs;
}

/**
 * @brief 获取平均回放倍速
 */
qreal PlaybackController::averageSpeed() const
{
    if (m_playCount == 0) {
        return 0.0;
    }
    return m_speedSum / static_cast<qreal>(m_playCount);
}

/**
 * @brief 重置统计数据
 */
void PlaybackController::resetStatistics()
{
    m_playCount = 0;
    m_totalPlayTimeMs = 0;
    m_speedSum = 0.0;
}
