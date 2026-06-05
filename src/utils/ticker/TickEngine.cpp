/**
 * @file TickEngine.cpp
 * @brief 精确节拍引擎实现
 */

#include "utils/ticker/TickEngine.h"

#include <QtMath>

TickEngine::TickEngine(QObject* parent)
    : QObject(parent), m_bpm(120.0), m_subdivision(1),
      m_tickCount(0), m_expectedNextMs(0.0), m_running(false),
      m_jitterSum(0.0)
{
    m_timer = new QTimer(this);
    m_timer->setTimerType(Qt::PreciseTimer);
    connect(m_timer, &QTimer::timeout, this, &TickEngine::onTimerTick);
}

TickEngine::~TickEngine()
{
    stop();
}

void TickEngine::setBpm(double bpm)
{
    m_bpm = qBound(20.0, bpm, 300.0);
    if (m_running) {
        m_timer->setInterval(static_cast<int>(tickIntervalMs()));
    }
}

void TickEngine::setSubdivision(int subdivision)
{
    m_subdivision = qBound(1, subdivision, 16);
    if (m_running) {
        m_timer->setInterval(static_cast<int>(tickIntervalMs()));
    }
}

void TickEngine::start()
{
    if (m_running) return;
    m_running = true;
    m_tickCount = 0;
    m_jitterSum = 0.0;
    m_stats.missedTicks = 0;
    m_stats.peakJitterMs = 0.0;

    m_elapsed.start();
    m_expectedNextMs = tickIntervalMs();
    m_timer->start(static_cast<int>(tickIntervalMs()));

    m_stats.totalStarts++;
}

void TickEngine::stop()
{
    if (!m_running) return;
    m_running = false;
    m_timer->stop();
}

bool TickEngine::isRunning() const { return m_running; }

double TickEngine::tickIntervalMs() const
{
    double beatMs = 60000.0 / m_bpm;
    return beatMs / m_subdivision;
}

void TickEngine::onTimerTick()
{
    double nowMs = static_cast<double>(m_elapsed.elapsed());
    double jitter = qAbs(nowMs - m_expectedNextMs);

    /* 累积抖动统计 */
    m_jitterSum += jitter;
    m_stats.totalTicks++;
    if (m_stats.totalTicks > 0) {
        m_stats.avgJitterMs = m_jitterSum / m_stats.totalTicks;
    }
    if (jitter > m_stats.peakJitterMs) {
        m_stats.peakJitterMs = jitter;
    }

    /* 漂移检测: 超过间隔10%视为漂移 */
    double interval = tickIntervalMs();
    if (jitter > interval * 0.1) {
        m_stats.missedTicks++;
        emit driftDetected(jitter);
    }

    emit tick(m_tickCount, nowMs);

    m_tickCount++;
    m_expectedNextMs += interval;

    /* 如果严重落后, 重置基准避免累积漂移 */
    if (nowMs - m_expectedNextMs > interval * 2) {
        m_expectedNextMs = nowMs + interval;
    }
}

void TickEngine::resetStatistics()
{
    m_stats = Stats{};
    m_jitterSum = 0.0;
}
