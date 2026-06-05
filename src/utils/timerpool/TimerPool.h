/**
 * @file TimerPool.h
 * @brief 定时器池 — 复用QTimer的高效定时管理
 *
 * 功能: 预分配QTimer对象池，避免频繁new/delete，
 *       支持一次性/周期性定时，统计复用率/活跃数。
 */
#ifndef TIMERPOOL_H
#define TIMERPOOL_H

#include <QObject>
#include <QTimer>
#include <QMap>
#include <QList>

/**
 * @class TimerPool
 * @brief 定时器对象池，减少QTimer创建销毁开销
 */
class TimerPool : public QObject {
    Q_OBJECT
public:
    /** 定时器统计 */
    struct Stats {
        quint64 totalAcquired = 0;
        quint64 totalReleased = 0;
        quint64 totalFired = 0;
        int     peakActive = 0;
        int     poolSize = 0;
        double  reuseRate = 0.0;
    };

    explicit TimerPool(int maxPoolSize = 50, QObject* parent = nullptr);

    /** 获取定时器 */
    QTimer* acquire(int intervalMs, bool singleShot = false);

    /** 释放定时器回池 */
    void release(QTimer* timer);

    /** 查询 */
    int activeCount() const;
    int availableCount() const;

    void setMaxPoolSize(int size);
    void clear();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void timerFired(QTimer* timer);
    void poolExhausted();

private:
    void onTimeout();

    QList<QTimer*> m_available;
    QMap<QTimer*, bool> m_active;
    int m_maxPoolSize;
    Stats m_stats;
};

#endif // TIMERPOOL_H
