/**
 * @file TimerPool.cpp
 * @brief 定时器池实现
 */

#include "utils/timerpool/TimerPool.h"

TimerPool::TimerPool(int maxPoolSize, QObject* parent)
    : QObject(parent), m_maxPoolSize(maxPoolSize) {}

QTimer* TimerPool::acquire(int intervalMs, bool singleShot)
{
    QTimer* timer = nullptr;

    if (!m_available.isEmpty()) {
        timer = m_available.takeLast();
        timer->setInterval(intervalMs);
        timer->setSingleShot(singleShot);
    } else {
        timer = new QTimer(this);
        timer->setInterval(intervalMs);
        timer->setSingleShot(singleShot);
        connect(timer, &QTimer::timeout, this, [this, timer]() {
            ++m_stats.totalFired;
            emit timerFired(timer);
        });
    }

    m_active[timer] = true;
    ++m_stats.totalAcquired;
    m_stats.poolSize = m_available.size() + m_active.size();

    if (m_active.size() > m_stats.peakActive) m_stats.peakActive = m_active.size();

    double totalOps = m_stats.totalAcquired + m_stats.totalReleased;
    m_stats.reuseRate = (totalOps > 0) ? static_cast<double>(m_stats.totalReleased) / totalOps : 0.0;

    return timer;
}

void TimerPool::release(QTimer* timer)
{
    if (!m_active.contains(timer)) return;

    timer->stop();
    m_active.remove(timer);

    if (m_available.size() < m_maxPoolSize) {
        m_available.append(timer);
    } else {
        delete timer;
    }

    ++m_stats.totalReleased;
    m_stats.poolSize = m_available.size() + m_active.size();

    double totalOps = m_stats.totalAcquired + m_stats.totalReleased;
    m_stats.reuseRate = (totalOps > 0) ? static_cast<double>(m_stats.totalReleased) / totalOps : 0.0;
}

int TimerPool::activeCount() const { return m_active.size(); }
int TimerPool::availableCount() const { return m_available.size(); }

void TimerPool::setMaxPoolSize(int size) { m_maxPoolSize = qMax(1, size); }

void TimerPool::clear()
{
    for (auto it = m_active.begin(); it != m_active.end(); ++it) {
        it.key()->stop();
        delete it.key();
    }
    m_active.clear();

    for (QTimer* t : m_available) delete t;
    m_available.clear();

    m_stats.poolSize = 0;
}

void TimerPool::resetStatistics() { m_stats = Stats{}; }
