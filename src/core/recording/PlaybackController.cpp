/**
 * @file PlaybackController.cpp
 * @brief 回放控制器实现
 */

#include "core/recording/PlaybackController.h"
#include <QTimer>

PlaybackController::PlaybackController(QObject* parent)
    : QObject(parent)
    , m_speed(1.0)
    , m_playing(false)
    , m_timer(new QTimer(this))
{
}

PlaybackController::~PlaybackController()
{
}

void PlaybackController::play()
{
    if (m_playing) {
        return;
    }
    m_playing = true;
    // TODO: 根据倍速计算定时器间隔并启动
    emit playbackStarted();
}

void PlaybackController::pause()
{
    if (!m_playing) {
        return;
    }
    m_playing = false;
    // TODO: 停止定时器
    emit playbackPaused();
}

void PlaybackController::stop()
{
    m_playing = false;
    // TODO: 停止定时器并重置进度
    emit playbackStopped();
}

void PlaybackController::setSpeed(qreal speed)
{
    if (qFuzzyCompare(m_speed, speed)) {
        return;
    }
    m_speed = speed;
    // TODO: 更新定时器间隔
    emit speedChanged(m_speed);
}

void PlaybackController::seekTo(qint64 timeMs)
{
    Q_UNUSED(timeMs)
    // TODO: 定位到指定时间点，更新内部进度
}

qreal PlaybackController::speed() const
{
    return m_speed;
}

bool PlaybackController::isPlaying() const
{
    return m_playing;
}
