/**
 * @file BinomialQueue.h
 * @brief 二项队列 — 二项树森林+合并链接+惰性删除+降键+优先队列
 *
 * 功能: 二项队列(Binomial Heap)实现可合并优先队列，支持森林管理、
 *       按度数合并链接、惰性删除、decrease-key操作、高效meld。
 *
 * 协作: StateTracker(状态优先级管理) / DataTrigger(触发优先级)
 */
#pragma once

#include <QObject>
#include <QElapsedTimer>
#include <QVector>
#include <QList>

/**
 * @brief 二项队列优先队列
 *
 * 二项队列是一组二项树的森林，支持O(log N)的插入/删除/查找最小，
 * 以及O(log N)的合并(meld)操作，适合需要频繁合并的优先队列场景。
 */
class BinomialQueue : public QObject {
    Q_OBJECT

public:
    /** @brief 操作结果 */
    struct OpResult {
        bool success = false;       ///< 操作是否成功
        double value = 0.0;        ///< 相关值
        int handle = -1;           ///< 元素句柄(用于decrease-key)
    };

    /** @brief 统计信息 */
    struct Stats {
        quint64 totalInsertions = 0;        ///< 累计插入次数
        quint64 totalDeletions = 0;         ///< 累计删除次数
        quint64 totalMerges = 0;            ///< 累计合并次数
        quint64 totalDecreaseKeys = 0;      ///< 累计降键次数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理时间(ms)
        int     currentSize = 0;            ///< 当前元素数
        int     peakSize = 0;               ///< 峰值元素数
    };

    explicit BinomialQueue(QObject* parent = nullptr);

    int insert(double value);
    OpResult findMin() const;
    OpResult deleteMin();
    void meld(BinomialQueue& other);

    int decreaseKey(int handle, double newValue);
    void remove(int handle);
    bool contains(int handle) const;
    bool isEmpty() const;
    int size() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void elementInserted(int handle, double value);
    void minDeleted(double value);
    void keyDecreased(int handle, double oldValue, double newValue);

private:
    /** @brief 二项树节点 */
    struct Node {
        double value = 0.0;        ///< 节点值
        int degree = 0;            ///< 度数(子树数量)
        int child = -1;            ///< 最左子节点索引
        int sibling = -1;          ///< 右兄弟索引
        int parent = -1;           ///< 父节点索引
        int handle = -1;           ///< 外部句柄
        bool marked = false;       ///< 惰性删除标记
    };

    int createNode(double value);
    int mergeTrees(int root1, int root2);
    void collectTrees(int root, QList<int>& trees) const;
    int findMinRoot() const;
    void bubbleUp(int nodeIdx);
    void cutFromParent(int nodeIdx);

    QVector<Node> m_nodes;             ///< 节点池
    QList<int> m_roots;                ///< 森林根列表
    int m_nextHandle;                  ///< 下一个句柄值
    QVector<int> m_handleToNode;       ///< 句柄到节点映射

    Stats m_stats;
    double m_timeSum = 0.0;
    mutable QElapsedTimer m_timer;
};
