/**
 * @file TimerWheel.h
 * @brief 时间轮调度器 — 高效分层定时器管理
 *
 * 功能: 单层时间轮实现高效定时器调度，支持一次性/周期定时器、
 *       取消/重置，统计定时器创建/触发/过期/耗时。
 */
#ifndef TIMERWHEEL_H
#define TIMERWHEEL_H

#include <QObject>
#include <QVector>
#include <QList>
#include <functional>

class TimerWheel : public QObject {
    Q_OBJECT
public:
    /** 统计 */
    struct Stats {
        quint64 totalTimersCreated = 0;
        quint64 totalTimersFired = 0;
        quint64 totalTimersCancelled = 0;
        quint64 totalTicks = 0;
    };

    /** 定时器句柄 */
    struct TimerHandle {
        int id = -1;
        int initialSlots = 0;
        bool periodic = false;
    };

    explicit TimerWheel(int wheelSize = 3600, int tickMs = 1000,
                         QObject* parent = nullptr);

    /** @brief 创建一次性定时器 @param delaySlots 延迟槽数 @param callback 回调 @return 定时器句柄 */
    TimerHandle scheduleOnce(int delaySlots, std::function<void()> callback);

    /** @brief 创建周期定时器 @param intervalSlots 间隔槽数 @param callback 回调 @return 定时器句柄 */
    TimerHandle schedulePeriodic(int intervalSlots,
                                  std::function<void()> callback);

    /** @brief 取消定时器 @param handle 定时器句柄 */
    void cancel(const TimerHandle& handle);

    /** @brief 推进时间轮 @param ticks 推进槽数 */
    void advance(int ticks = 1);

    /** @brief 当前槽位 */
    int currentSlot() const { return m_currentSlot; }

    int wheelSize() const { return m_wheelSize; }
    int tickMs() const { return m_tickMs; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void timerFired(int timerId);
    void wheelAdvanced(int newSlot);

private:
    struct TimerEntry {
        int id;
        int rounds;
        int interval;
        bool periodic;
        bool cancelled;
        std::function<void()> callback;
    };

    int m_wheelSize;
    int m_tickMs;
    int m_currentSlot;
    int m_nextId;
    QVector<QList<TimerEntry>> m_wheel;
    Stats m_stats;
};

#endif // TIMERWHEEL_H
