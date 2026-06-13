/**
 * @file Treap18.h
 * @brief 树堆(手指树增强与分裂连接双端队列实现支持顺序统计查询的高效序列操作) — Treap with Finger Tree Augmentation and Split-Join Deques for Efficient Sequence Operations with Order-Statistic Queries
 *
 * 功能: 实现树堆(treap)，采用手指树增强(finger tree augmentation)
 *       与分裂连接双端队列(split-join deques)实现支持顺序统计查询的高效序列操作(efficient sequence operations with order-statistic queries)。
 *
 * 协作: AA12(AA树) / RedBlackTree(红黑树) / SplayTree(伸展树)
 */
#pragma once

#include <QObject>
#include <QVector>

class Treap18 : public QObject {
    Q_OBJECT

public:
    /** @brief Treap node with finger tree augmentation */
    struct Node {
        int key = 0;
        double value = 0.0;
        int priority = 0;       // Heap priority (random)
        int left = -1;          // Index into node pool
        int right = -1;
        int parent = -1;
        int size = 1;           // Subtree size for order statistics
        double aggregate = 0.0; // Monoid aggregate (sum for finger tree)
    };

    /** @brief Search result */
    struct SearchResult {
        bool found = false;
        double value = 0.0;
        int rank = 0;           // Order statistic rank
        double elapsedMs = 0.0;
    };

    /** @brief Range query result */
    struct RangeResult {
        QVector<QPair<int, double>> entries;
        double prefixSum = 0.0;
        double elapsedMs = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalInserts = 0;
        quint64 totalDeletes = 0;
        quint64 totalQueries = 0;
        int nodeCount = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Treap18(QObject *parent = nullptr);
    ~Treap18() override;

    /** @brief Insert key-value pair */
    bool insert(int key, double value);

    /** @brief Remove key */
    bool remove(int key);

    /** @brief Search by key with rank */
    SearchResult search(int key) const;

    /** @brief Select k-th order statistic */
    SearchResult select(int k) const;

    /** @brief Split treap at position into two sequences */
    QPair<int, int> split(int root, int key);

    /** @brief Join two treaps */
    int join(int left, int right);

    /** @brief Range query with prefix sum */
    RangeResult rangeQuery(int lo, int hi) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void insertDone(int key, int nodeCount, double timeMs);
    void removeDone(int key, bool success, double timeMs);

private:
    QVector<Node> m_nodes;
    int m_root = -1;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Update subtree size and aggregate */
    void update(int idx);

    /** @brief Get subtree size safely */
    int sz(int idx) const;

    /** @brief Get subtree aggregate safely */
    double aggr(int idx) const;

    /** @brief Rotate right */
    int rotateRight(int idx);

    /** @brief Rotate left */
    int rotateLeft(int idx);

    /** @brief Heapify up to maintain heap property */
    int heapifyUp(int idx);

    /** @brief Recursive insert */
    int insertHelper(int root, int key, double value);

    /** @brief Recursive delete */
    int removeHelper(int root, int key);

    /** @brief In-order traversal */
    void inOrder(int idx, QVector<QPair<int, double>>& result) const;

    /** @brief Range query helper */
    void rangeHelper(int idx, int lo, int hi, QVector<QPair<int, double>>& result) const;
};
