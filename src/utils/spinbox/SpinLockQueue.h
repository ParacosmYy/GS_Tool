/**
 * @file SpinLockQueue.h
 * @brief 自旋锁队列 — 低延迟的线程安全队列
 *
 * 功能: 基于自旋锁的线程安全FIFO队列，适用于高频低延迟场景，
 *       统计入队/出队次数/队列峰值/竞争次数。
 */
#ifndef SPINLOCKQUEUE_H
#define SPINLOCKQUEUE_H

#include <QObject>
#include <QVector>
#include <atomic>
#include <mutex>

/**
 * @class SpinLockQueue
 * @brief 高性能线程安全队列，使用自旋锁保护
 */
class SpinLockQueue : public QObject {
    Q_OBJECT
public:
    /** 队列统计 */
    struct Stats {
        quint64 totalPushes = 0;
        quint64 totalPops = 0;
        int     peakSize = 0;
        quint64 contentionCount = 0;
        quint64 totalBytesPushed = 0;
    };

    explicit SpinLockQueue(int capacity = 4096, QObject* parent = nullptr);

    /** 入队 */
    bool push(const QByteArray& item);
    bool push(const QVector<QByteArray>& items);

    /** 出队 */
    QByteArray pop();
    QVector<QByteArray> popAll();

    /** 查询 */
    int size() const;
    bool isEmpty() const;
    void clear();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void dataAvailable(int queueSize);

private:
    QVector<QByteArray> m_buffer;
    std::atomic<int> m_head;
    std::atomic<int> m_tail;
    std::atomic<int> m_count;
    int m_capacity;
    std::atomic<uint64_t> m_spinCount;
    Stats m_stats;
};

#endif // SPINLOCKQUEUE_H
