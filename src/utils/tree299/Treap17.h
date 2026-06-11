/**
 * @file Treap17.h
 * @brief 树堆(确定性优先级哈希与指针搜索实现局部性感知顺序访问模式优化) — Treap with Deterministic Priority Hashing and Finger Search for Locality-aware Sequential Access Pattern Optimization
 *
 * 功能: 实现树堆(Treap)，采用确定性优先级哈希(deterministic priority hashing)
 *       与指针搜索(finger search)实现局部性感知顺序访问模式优化(locality-aware sequential access pattern optimization)。
 *
 * 协作: WeightBalancedTree12(权重平衡树) / SplayTree14(伸展树) / RedBlackTree(红黑树)
 */
#pragma once

#include <QObject>
#include <QVector>

class Treap17 : public QObject {
    Q_OBJECT

public:
    /** @brief Treap node stored in array for cache locality */
    struct Node {
        double key = 0.0;
        quint64 priority = 0;   // deterministic hash-based priority
        int left = -1;
        int right = -1;
        int parent = -1;
        int subtreeSize = 1;
    };

    /** @brief Search result with finger hint */
    struct SearchResult {
        bool found = false;
        int nodeIndex = -1;
        int depth = 0;
        int comparisons = 0;
        bool fingerUsed = false;
    };

    /** @brief Range query result */
    struct RangeResult {
        QVector<double> keys;
        int count = 0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalInserts = 0;
        quint64 totalDeletes = 0;
        quint64 totalSearches = 0;
        int treeSize = 0;
        int treeHeight = 0;
        int fingerHits = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Treap17(QObject *parent = nullptr);
    ~Treap17() override;

    /** @brief Insert a key with deterministic priority */
    bool insert(double key);

    /** @brief Remove a key */
    bool remove(double key);

    /** @brief Search with finger optimization for sequential access */
    SearchResult search(double key);

    /** @brief Range query [lo, hi] */
    RangeResult rangeQuery(double lo, double hi) const;

    /** @brief In-order traversal */
    QVector<double> inOrder() const;

    /** @brief Set finger position for locality-aware search */
    void setFinger(double key);

    const QVector<Node>& nodes() const { return m_nodes; }
    int root() const { return m_root; }
    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void insertDone(double key, int size, double timeMs);
    void removeDone(double key, int size, double timeMs);

private:
    QVector<Node> m_nodes;
    int m_root = -1;
    int m_finger = -1;         // last accessed node index
    double m_fingerKey = 0.0;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Deterministic priority via hash of key */
    quint64 hashPriority(double key) const;

    /** @brief Allocate new node */
    int allocNode(double key);

    /** @brief Rotate node with left child (right rotation) */
    int rotateRight(int idx);

    /** @brief Rotate node with right child (left rotation) */
    int rotateLeft(int idx);

    /** @brief Maintain heap property bottom-up from idx */
    void heapifyUp(int idx);

    /** @brief Maintain heap property top-down from idx */
    void heapifyDown(int idx);

    /** @brief Update subtree sizes along path */
    void updateSizes(int idx);

    /** @brief Finger search: start from last accessed node */
    SearchResult fingerSearch(double key);

    /** @brief Standard root-to-leaf search */
    SearchResult rootSearch(double key) const;

    /** @brief Recursive in-order helper */
    void inOrderHelper(int idx, QVector<double>& result) const;

    /** @brief Recursive range helper */
    void rangeHelper(int idx, double lo, double hi, QVector<double>& result) const;
};
