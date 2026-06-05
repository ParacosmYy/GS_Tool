/**
 * @file SlidingWindowCounter.h
 * @brief 滑动窗口计数器 — 时间窗口内事件频率统计
 *
 * 功能: 基于时间槽的滑动窗口事件计数，支持多粒度窗口、
 *       频率限制、突发检测，统计事件数/窗口数/突发次数。
 */
#ifndef SLIDINGWINDOWCOUNTER_H
#define SLIDINGWINDOWCOUNTER_H

#include <QObject>
#include <QVector>
#include <QElapsedTimer>

class SlidingWindowCounter : public QObject {
    Q_OBJECT
public:
    /** 统计 */
    struct Stats {
        quint64 totalEvents = 0;
        quint64 totalWindows = 0;
        quint64 totalBursts = 0;
        double  peakRate = 0.0;
    };

    explicit SlidingWindowCounter(int windowSlots = 60,
                                   qint64 slotMs = 1000,
                                   QObject* parent = nullptr);

    /** @brief 记录事件 */
    void recordEvent();

    /** @brief 记录多个事件 @param count 事件数 */
    void recordEvents(int count);

    /** @brief 当前窗口事件数 @return 事件数 */
    int currentCount() const;

    /** @brief 当前速率(事件/秒) @return 速率 */
    double currentRate() const;

    /** @brief 设置突发阈值 @param threshold 事件数 */
    void setBurstThreshold(int threshold);

    /** @brief 推进时间 @param elapsedMs 经过毫秒数 */
    void advance(qint64 elapsedMs);

    int windowSlots() const { return m_slots.size(); }
    qint64 slotMs() const { return m_slotMs; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();
    void reset();

signals:
    void burstDetected(int count, int threshold);

private:
    qint64 currentSlot() const;
    void advanceToSlot(qint64 targetSlot);

    QVector<int> m_slots;
    qint64 m_slotMs;
    qint64 m_currentSlot;
    int m_currentIdx;
    int m_burstThreshold;
    QElapsedTimer m_timer;
    Stats m_stats;
};

#endif // SLIDINGWINDOWCOUNTER_H
