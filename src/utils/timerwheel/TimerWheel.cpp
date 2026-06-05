/**
 * @file TimerWheel.cpp
 * @brief 时间轮调度器实现
 */

#include "utils/timerwheel/TimerWheel.h"

#include <algorithm>

TimerWheel::TimerWheel(int wheelSize, int tickMs, QObject* parent)
    : QObject(parent), m_wheelSize(qMax(8, wheelSize)),
      m_tickMs(qMax(1, tickMs)), m_currentSlot(0), m_nextId(1)
{
    m_wheel.resize(m_wheelSize);
}

TimerWheel::TimerHandle TimerWheel::scheduleOnce(
    int delaySlots, std::function<void()> callback)
{
    TimerHandle handle;
    handle.id = m_nextId++;
    handle.initialSlots = delaySlots;
    handle.periodic = false;

    int targetSlot = (m_currentSlot + delaySlots) % m_wheelSize;

    TimerEntry entry;
    entry.id = handle.id;
    entry.rounds = delaySlots / m_wheelSize;
    entry.interval = 0;
    entry.periodic = false;
    entry.cancelled = false;
    entry.callback = std::move(callback);

    m_wheel[targetSlot].append(entry);
    m_stats.totalTimersCreated++;

    return handle;
}

TimerWheel::TimerHandle TimerWheel::schedulePeriodic(
    int intervalSlots, std::function<void()> callback)
{
    TimerHandle handle;
    handle.id = m_nextId++;
    handle.initialSlots = intervalSlots;
    handle.periodic = true;

    int targetSlot = (m_currentSlot + intervalSlots) % m_wheelSize;

    TimerEntry entry;
    entry.id = handle.id;
    entry.rounds = intervalSlots / m_wheelSize;
    entry.interval = intervalSlots;
    entry.periodic = true;
    entry.cancelled = false;
    entry.callback = std::move(callback);

    m_wheel[targetSlot].append(entry);
    m_stats.totalTimersCreated++;

    return handle;
}

void TimerWheel::cancel(const TimerHandle& handle)
{
    for (auto& slot : m_wheel) {
        for (auto& entry : slot) {
            if (entry.id == handle.id) {
                entry.cancelled = true;
                m_stats.totalTimersCancelled++;
                return;
            }
        }
    }
}

void TimerWheel::advance(int ticks)
{
    for (int t = 0; t < ticks; ++t) {
        QList<TimerEntry> currentEntries;
        currentEntries.swap(m_wheel[m_currentSlot]);

        for (auto& entry : currentEntries) {
            if (entry.cancelled) continue;

            if (entry.rounds > 0) {
                entry.rounds--;
                m_wheel[m_currentSlot].append(entry);
                continue;
            }

            /* 触发 */
            if (entry.callback) entry.callback();
            m_stats.totalTimersFired++;
            emit timerFired(entry.id);

            /* 周期定时器重新调度 */
            if (entry.periodic) {
                int targetSlot = (m_currentSlot + entry.interval) % m_wheelSize;
                entry.rounds = entry.interval / m_wheelSize;
                m_wheel[targetSlot].append(entry);
            }
        }

        m_currentSlot = (m_currentSlot + 1) % m_wheelSize;
    }

    m_stats.totalTicks += ticks;
    emit wheelAdvanced(m_currentSlot);
}

void TimerWheel::resetStatistics()
{
    m_stats = Stats{};
}
