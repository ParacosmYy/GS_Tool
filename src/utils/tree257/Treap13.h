/**
 * @file Treap13.h
 * @brief 树堆(Zip树优先级几何分布+合并-分裂区间操作) — Treap with Zip Tree Priority Scheme Using Geometric Distribution and Merge-with-Split for Interval Operations
 *
 * 功能: 实现树堆(Treap)，采用Zip树优先级方案(zip tree priority scheme)
 *       使用几何分布(geometric distribution)生成优先级，支持合并-分裂
 *       (merge-with-split)区间操作(interval operations)。
 *
 * 协作: AVLTree8(AVL树) / RedBlackTree10(红黑树) / WeightBalancedTree9(权重平衡树)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 树堆(Zip树优先级+几何分布+合并-分裂区间)
 */
class Treap13 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numNodes = 0;
        int numInserts = 0;
        int numDeletes = 0;
        int numSplits = 0;
        int numMerges = 0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Treap node */
    struct Node {
        int key = 0;
        int priority = 0;    // Geometric-distribution rank
        int size = 1;        // Subtree size for order statistics
        int left = -1;
        int right = -1;
    };

    /** @brief Interval query result */
    struct IntervalResult {
        QVector<int> keys;
        int count = 0;
    };

    explicit Treap13(QObject *parent = nullptr);
    ~Treap13() override;

    /** @brief Set geometric distribution parameter p */
    void setGeoParam(double p);

    /** @brief Insert a key with zip-tree priority */
    void insert(int key);

    /** @brief Remove a key */
    void remove(int key);

    /** @brief Check if key exists */
    bool contains(int key) const;

    /** @brief Query all keys in range [lo, hi] */
    IntervalResult queryRange(int lo, int hi) const;

    /** @brief Split tree at key into (<key) and (>=key) */
    void splitAt(int key, Treap13& left, Treap13& right);

    /** @brief Merge two treaps (all keys in left < all keys in right) */
    void mergeWith(Treap13& other);

    /** @brief In-order traversal */
    QVector<int> inOrder() const;

    /** @brief Get tree size */
    int size() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void treeUpdated(int numNodes, int height, double timeMs);

private:
    double m_geoP = 0.5;
    int m_root = -1;
    QVector<Node> m_nodes;
    int m_freeList = -1;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Generate geometric-distribution priority */
    int generatePriority() const;

    /** @brief Allocate node */
    int allocNode(int key, int priority);

    /** @brief Free node */
    void freeNode(int idx);

    /** @brief Update subtree size */
    void updateSize(int idx);

    /** @brief Get node size (safe) */
    int nodeSize(int idx) const;

    /** @brief Merge two subtrees (all keys in a < all keys in b) */
    int merge(int a, int b);

    /** @brief Split subtree at key */
    void split(int root, int key, int& left, int& right);

    /** @brief Find key index */
    int find(int key) const;

    /** @brief In-order helper */
    void inOrderHelper(int idx, QVector<int>& result) const;

    /** @brief Range query helper */
    void rangeQuery(int idx, int lo, int hi, QVector<int>& result) const;

    /** @brief Get tree height */
    int heightHelper(int idx) const;
};
