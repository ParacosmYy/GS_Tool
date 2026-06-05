/**
 * @file FibonacciHeap.h
 * @brief 斐波那契堆 — 优先队列的高级数据结构
 *
 * 功能: 斐波那契堆实现，支持insert/extractMin/decreaseKey/merge，
 *       统计操作次数/节点数/耗时。
 */
#ifndef FIBONACCIHEAP_H
#define FIBONACCIHEAP_H

#include <QObject>
#include <QVector>

class FibonacciHeap : public QObject {
    Q_OBJECT
public:
    /** 统计 */
    struct Stats {
        quint64 totalInserts = 0;
        quint64 totalExtracts = 0;
        quint64 totalDecreaseKeys = 0;
        quint64 totalMerges = 0;
        double  avgProcessingTimeMs = 0.0;
    };

    explicit FibonacciHeap(QObject* parent = nullptr);
    ~FibonacciHeap();

    /** @brief 插入元素 @param key 键值 @return 节点句柄 */
    int insert(double key);

    /** @brief 获取最小值 @return 最小键值 */
    double findMin() const;

    /** @brief 删除并返回最小值 @return 最小键值 */
    double extractMin();

    /** @brief 降低键值 @param handle 节点句柄 @param newKey 新键值 */
    void decreaseKey(int handle, double newKey);

    /** @brief 合并另一个堆 @param other 另一个斐波那契堆 */
    void merge(FibonacciHeap& other);

    bool isEmpty() const { return m_minIndex < 0; }
    int size() const { return m_size; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void minExtracted(double key);
    void keyDecreased(int handle, double oldKey, double newKey);

private:
    struct Node {
        double key;
        int parent;
        int child;
        int left;
        int right;
        int degree;
        bool mark;
    };

    void consolidate();
    void link(int childIdx, int parentIdx);
    void cut(int childIdx, int parentIdx);
    void cascadingCut(int idx);

    QVector<Node> m_nodes;
    int m_minIndex;
    int m_size;
    Stats m_stats;
    double m_timeSum;
};

#endif // FIBONACCIHEAP_H
