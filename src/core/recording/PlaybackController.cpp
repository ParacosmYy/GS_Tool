/**
 * @file PlaybackController.cpp
 * @brief 回放控制器实现 - 播放/暂停/停止及定时器驱动
 *
 * 时间累积模型：m_currentTimeMs 为已累积的基准时间，
 * m_elapsed 测量自上次累积以来的真实经过时间，
 * 当前播放位置 = m_currentTimeMs + elapsed * m_speed。
 *
 * 倍速控制/定位/统计接口见 PlaybackControllerSpeed.cpp
 */

#include "core/recording/PlaybackController.h"
#include <QTimer>

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

    // 统计：累计回放启动计数，若从暂停恢复则额外计数
    ++m_totalPlaybacks;
    if (m_currentTimeMs > 0) {
        ++m_totalPlaybackResumes;
    }

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
 * 状态查询
 * ============================================================ */

/** @brief 查询是否正在回放 @return true表示正在回放 */
bool PlaybackController::isPlaying() const
{
    return m_playing;
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
        ++m_totalCompletions;
        m_totalPlayTimeMs += m_durationMs;

        emit playbackFinished();
        emit playbackStopped();
    } else {
        emit timeUpdated(elapsed);
    }
}
