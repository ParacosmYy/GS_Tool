/**
 * @file SlidingWindowCounter.cpp
 * @brief 滑动窗口计数器实现
 */

#include "utils/slidingwindow3/SlidingWindowCounter.h"

#include <QtMath>

SlidingWindowCounter::SlidingWindowCounter(int windowSlots, qint64 slotMs,
                                             QObject* parent)
    : QObject(parent), m_slots(windowSlots, 0),
      m_slotMs(qMax(1, slotMs)), m_currentSlot(0),
      m_currentIdx(0), m_burstThreshold(0)
{
    m_timer.start();
}

void SlidingWindowCounter::recordEvent()
{
    advanceToSlot(currentSlot());
    m_slots[m_currentIdx]++;
    m_stats.totalEvents++;

    if (m_burstThreshold > 0 && currentCount() > m_burstThreshold) {
        m_stats.totalBursts++;
        emit burstDetected(currentCount(), m_burstThreshold);
    }
}

void SlidingWindowCounter::recordEvents(int count)
{
    advanceToSlot(currentSlot());
    m_slots[m_currentIdx] += count;
    m_stats.totalEvents += count;

    if (m_burstThreshold > 0 && currentCount() > m_burstThreshold) {
        m_stats.totalBursts++;
        emit burstDetected(currentCount(), m_burstThreshold);
    }
}

int SlidingWindowCounter::currentCount() const
{
    int total = 0;
    for (int c : m_slots) total += c;
    return total;
}

double SlidingWindowCounter::currentRate() const
{
    double windowMs = m_slots.size() * m_slotMs;
    return (windowMs > 0) ? currentCount() * 1000.0 / windowMs : 0.0;
}

void SlidingWindowCounter::setBurstThreshold(int threshold)
{
    m_burstThreshold = qMax(0, threshold);
}

void SlidingWindowCounter::advance(qint64 elapsedMs)
{
    qint64 targetSlot = elapsedMs / m_slotMs;
    advanceToSlot(targetSlot);
}

qint64 SlidingWindowCounter::currentSlot() const
{
    return m_timer.elapsed() / m_slotMs;
}

void SlidingWindowCounter::advanceToSlot(qint64 targetSlot)
{
    while (m_currentSlot < targetSlot) {
        m_currentSlot++;
        m_currentIdx = (m_currentIdx + 1) % m_slots.size();
        m_slots[m_currentIdx] = 0;
        m_stats.totalWindows++;
    }

    double rate = currentRate();
    if (rate > m_stats.peakRate) m_stats.peakRate = rate;
}

void SlidingWindowCounter::reset()
{
    m_slots.fill(0);
    m_currentSlot = 0;
    m_currentIdx = 0;
    m_timer.restart();
}

void SlidingWindowCounter::resetStatistics()
{
    m_stats = Stats{};
}
