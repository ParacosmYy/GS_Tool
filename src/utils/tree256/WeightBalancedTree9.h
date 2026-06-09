/**
 * @file WeightBalancedTree9.h
 * @brief 权重平衡树(秩平衡不变量+批量更新全局重建阈值) — Weight-Balanced Tree with Rank-Balanced Invariant and Batch-Update Rebalancing via Global Rebuild Threshold
 *
 * 功能: 实现权重平衡树(Weight-Balanced Tree)，维护秩平衡不变量
 *       (rank-balanced invariant)保证O(log n)操作，支持批量更新
 *       (batch update)触发全局重建(global rebuild)当失衡超过阈值。
 *
 * 协作: AVLTree8(AVL树) / RedBlackTree10(红黑树) / SplayTree11(伸展树)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 权重平衡树(秩平衡不变量+批量更新全局重建)
 */
class WeightBalancedTree9 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numNodes = 0;
        int numInserts = 0;
        int numDeletes = 0;
        int numRebalances = 0;
        int numGlobalRebuilds = 0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Tree node */
    struct Node {
        int key = 0;
        int weight = 1;         // Subtree size (rank)
        int height = 1;
        int left = -1;
        int right = -1;
        int parent = -1;
    };

    explicit WeightBalancedTree9(QObject *parent = nullptr);
    ~WeightBalancedTree9() override;

    /** @brief Set alpha balance parameter (0.25..0.5) */
    void setAlpha(double alpha);

    /** @brief Set batch size threshold for global rebuild */
    void setRebuildThreshold(int threshold);

    /** @brief Insert a key */
    void insert(int key);

    /** @brief Remove a key */
    void remove(int key);

    /** @brief Check if key exists */
    bool contains(int key) const;

    /** @brief Batch insert multiple keys */
    void batchInsert(const QVector<int>& keys);

    /** @brief In-order traversal */
    QVector<int> inOrder() const;

    /** @brief Get tree height */
    int height() const;

    /** @brief Get number of nodes */
    int size() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void treeUpdated(int numNodes, int height, double timeMs);

private:
    double m_alpha = 0.35;
    int m_rebuildThreshold = 100;
    int m_batchOpsSinceRebuild = 0;

    QVector<Node> m_nodes;     // Node pool
    int m_root = -1;
    int m_freeList = -1;       // Free node list head

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Allocate a new node */
    int allocNode(int key);

    /** @brief Free a node to free list */
    void freeNode(int idx);

    /** @brief Get weight of subtree */
    int weight(int idx) const;

    /** @brief Update weight and height of node */
    void updateNode(int idx);

    /** @brief Check if node is alpha-weight-balanced */
    bool isBalanced(int idx) const;

    /** @brief Find the deepest unbalanced ancestor */
    int findUnbalanced(int start) const;

    /** @brief Rebalance subtree at given root */
    int rebalance(int root);

    /** @brief Flatten subtree to sorted array */
    void flatten(int idx, QVector<int>& keys) const;

    /** @brief Build balanced tree from sorted keys */
    int buildBalanced(const QVector<int>& keys, int lo, int hi);

    /** @brief Global rebuild of entire tree */
    void globalRebuild();

    /** @brief Search for key, return node index */
    int find(int key) const;

    /** @brief Find minimum key node in subtree */
    int findMin(int idx) const;

    /** @brief Recursive in-order traversal */
    void inOrderHelper(int idx, QVector<int>& result) const;
};
