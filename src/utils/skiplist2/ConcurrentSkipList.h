/**
 * @file ConcurrentSkipList.h
 * @brief 并发跳表 — 细粒度锁定的有序并发容器
 *
 * 功能: 并发安全的跳表，读操作无锁，写操作使用细粒度节点锁，
 *       支持插入/删除/查找/范围查询，统计操作次数/耗时。
 */
#ifndef CONCURRENTSKIPLIST_H
#define CONCURRENTSKIPLIST_H

#include <QObject>
#include <QVector>
#include <QMutex>
#include <QMutexLocker>
#include <QRandomGenerator>
#include <memory>

class ConcurrentSkipList : public QObject {
    Q_OBJECT
public:
    /** 统计 */
    struct Stats {
        quint64 totalInsertions = 0;
        quint64 totalDeletions = 0;
        quint64 totalLookups = 0;
        int     maxHeight = 0;
        int     elementCount = 0;
        double  avgProcessingTimeMs = 0.0;
    };

    explicit ConcurrentSkipList(int maxLevel = 16,
                                 QObject* parent = nullptr);
    ~ConcurrentSkipList();

    /** @brief 插入值 @param value 值 @return 是否成功(已存在返回false) */
    bool insert(double value);

    /** @brief 删除值 @param value 值 @return 是否成功 */
    bool remove(double value);

    /** @brief 查找值 @param value 值 @return 是否存在 */
    bool contains(double value);

    /** @brief 范围查询 [min, max] @param minVal 最小值 @param maxVal 最大值 @return 值列表 */
    QVector<double> rangeQuery(double minVal, double maxVal) const;

    /** @brief 获取所有元素(排序) @return 值列表 */
    QVector<double> toSortedList() const;

    /** @brief 元素数 */
    int size() const { return m_count; }

    /** @brief 是否为空 */
    bool isEmpty() const { return m_count == 0; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void valueInserted(double value);
    void valueRemoved(double value);

private:
    int randomLevel() const;

    struct Node {
        double value;
        int level;
        QVector<Node*> forward;  ///< 每层的下一个节点
        QMutex lock;

        Node(double v, int lvl) : value(v), level(lvl),
            forward(lvl + 1, nullptr) {}
    };

    int m_maxLevel;
    int m_currentLevel;
    Node* m_head;
    int m_count;
    mutable QMutex m_globalMutex;

    Stats m_stats;
    double m_timeSum;
};

#endif // CONCURRENTSKIPLIST_H
