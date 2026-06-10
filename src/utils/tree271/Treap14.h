/**
 * @file Treap14.h
 * @brief 树堆(随机堆优先级与分裂合并操作高效有序集范围查询) — Treap with Randomized Heap Priority and Split-Merge Operations for Efficient Ordered-Set and Range Queries
 *
 * 功能: 实现树堆(Treap)，采用随机堆优先级(randomized heap priority)
 *       与分裂合并操作(split-merge operations)实现高效有序集范围查询(ordered-set and range queries)。
 *
 * 协作: WeightBalancedTree10(权重平衡树) / SplayTree12(伸展树) / AVLTree10(AVL树)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 树堆(随机堆优先级与分裂合并操作高效有序集范围查询)
 */
class Treap14 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int treeSize = 0;
        int treeHeight = 0;
        int numRotations = 0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Tree node stored in pool */
    struct Node {
        double key = 0.0;
        int value = 0;
        int priority = 0;       // random heap priority
        int size = 1;           // subtree size for rank queries
        int left = -1;
        int right = -1;
    };

    explicit Treap14(QObject *parent = nullptr);
    ~Treap14() override;

    /** @brief Insert key-value pair */
    void insert(double key, int value = 0);

    /** @brief Remove a key, returns true if found */
    bool remove(double key);

    /** @brief Search for key, returns value (-1 if not found) */
    int search(double key) const;

    /** @brief Get k-th smallest key (by rank) */
    double kth(int k) const;

    /** @brief Count keys less than given key */
    int rank(double key) const;

    /** @brief Count keys in range [lo, hi] */
    int rangeCount(double lo, double hi) const;

    /** @brief In-order traversal */
    QVector<double> inOrderKeys() const;

    /** @brief Get tree height */
    int height() const;

    /** @brief Get node count */
    int size() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void treeUpdated(int size, int height, double timeMs);

private:
    int m_root = -1;
    int m_rotations = 0;
    QVector<Node> m_nodes;
    QVector<int> m_freeList;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Allocate a new node */
    int allocNode(double key, int value);

    /** @brief Free a node */
    void freeNode(int idx);

    /** @brief Update subtree size */
    void updateSize(int idx);

    /** @brief Get subtree size safely */
    int nodeSize(int idx) const;

    /** @brief Split treap by key: left has keys < key, right has keys >= key */
    QPair<int, int> split(int idx, double key);

    /** @brief Merge two treaps (all keys in a < all keys in b) */
    int merge(int a, int b);

    /** @brief Recursive height */
    int heightHelper(int idx) const;

    /** @brief In-order helper */
    void inOrderHelper(int idx, QVector<double>& result) const;

    /** @brief Rank helper: count keys < key in subtree */
    int rankHelper(int idx, double key) const;
};
