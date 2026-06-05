/**
 * @file CircularPriorityQueue.h
 * @brief 环形优先队列 — 固定容量的高效优先级管理
 *
 * 功能: 基于环形缓冲区的优先队列，支持插入/弹出/peek/批量操作，
 *       统计操作次数/容量利用率/耗时。
 */
#ifndef CIRCULARPRIORITYQUEUE_H
#define CIRCULARPRIORITYQUEUE_H

#include <QObject>
#include <QVector>

template<typename T>
class CircularPriorityQueue : public QObject {
    /* QObject不支持模板Q_OBJECT，改为普通类+Stats */
};

/**
 * @class CircularPriorityQueue
 * @brief 固定容量环形优先队列(double特化)
 */
class CprioQueue : public QObject {
    Q_OBJECT
public:
    /** 统计 */
    struct Stats {
        quint64 totalPushes = 0;
        quint64 totalPops = 0;
        quint64 totalOverflows = 0;
        double  avgProcessingTimeMs = 0.0;
    };

    explicit CprioQueue(int capacity = 256, QObject* parent = nullptr);

    /** @brief 插入元素 @param priority 优先级 @param value 值 @return 是否成功(满时丢弃) */
    bool push(double priority, double value);

    /** @brief 弹出最高优先级 @param[out] priority 优先级 @param[out] value 值 @return 是否成功 */
    bool pop(double& priority, double& value);

    /** @brief 查看最高优先级 @return (priority, value) */
    QPair<double, double> peek() const;

    /** @brief 批量弹出 @param maxCount 最大数量 @return 弹出列表 */
    QVector<QPair<double, double>> popBatch(int maxCount);

    int size() const;
    int capacity() const;
    bool isEmpty() const;
    bool isFull() const;
    void clear();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void overflowDiscarded(double priority);
    void elementPopped(double priority);

private:
    void heapifyUp(int idx);
    void heapifyDown(int idx);

    QVector<QPair<double, double>> m_heap;
    int m_capacity;
    int m_size;
    Stats m_stats;
    double m_timeSum;
};

#endif // CIRCULARPRIORITYQUEUE_H
