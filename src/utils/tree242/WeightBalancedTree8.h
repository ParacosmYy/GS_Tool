/**
 * @file WeightBalancedTree8.h
 * @brief 权重平衡树(秩平衡重平衡+部分重建O(log n)最坏情况操作) — Weight-Balanced Tree with Rank-Balanced Rebalancing and Partial Rebuilding for O(log n) Worst-Case Operations
 *
 * 功能: 实现权重平衡树(Weight-balanced tree)，采用秩平衡重平衡(rank-balanced rebalancing)
 *       通过跟踪子树权重(节点数)检测不平衡条件，结合部分重建(partial rebuilding)策略
 *       对失衡子树进行局部重建，保证O(log n)最坏情况查找/插入/删除操作。
 *
 * 协作: CartesianTree9(笛卡尔树) / AVLTree7(AVL树) / RedBlackTree6(红黑树)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 权重平衡树(秩平衡重平衡+部分重建O(log n)最坏情况操作)
 */
class WeightBalancedTree8 : public QObject {
    Q_OBJECT

public:
    /** @brief Tree node */
    struct Node {
        double key = 0.0;
        int value = 0;
        int left = -1;
        int right = -1;
        int weight = 1;       // subtree size
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int treeSize = 0;
        int treeHeight = 0;
        int numRebalances = 0;
        int numPartialRebuilds = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit WeightBalancedTree8(QObject *parent = nullptr);
    ~WeightBalancedTree8() override;

    /** @brief Insert key-value pair */
    void insert(double key, int value);

    /** @brief Remove key */
    bool remove(double key);

    /** @brief Search for key, returns value or -1 */
    int search(double key) const;

    /** @brief In-order traversal of keys */
    QVector<double> inOrderKeys() const;

    /** @brief Get tree height */
    int height() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void insertCompleted(double key, int height, double timeMs);
    void rebalancePerformed(int subtreeSize, int newHeight);

private:
    QVector<Node> m_nodes;
    int m_root = -1;

    // Balance factor threshold (alpha * child weight < other child weight triggers rebalance)
    static constexpr double kAlpha = 0.29;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Allocate a new node, returns index */
    int allocNode(double key, int value);

    /** @brief Update weight of node */
    void updateWeight(int idx);

    /** @brief Check balance condition */
    bool isBalanced(int idx) const;

    /** @brief Rebalance subtree rooted at idx via partial rebuilding */
    int rebalance(int idx);

    /** @brief Partial rebuild: collect, rebuild balanced subtree */
    int partialRebuild(int idx);

    /** @brief Collect subtree keys in-order */
    void collectInOrder(int idx, QVector<int>& indices) const;

    /** @brief Build balanced subtree from sorted indices */
    int buildBalanced(const QVector<int>& indices, int lo, int hi);

    /** @brief Recursive insert helper */
    int insertHelper(int idx, double key, int value, bool& inserted);

    /** @brief Recursive remove helper */
    int removeHelper(int idx, double key, bool& removed);

    /** @brief Find minimum key node in subtree */
    int findMin(int idx) const;

    /** @brief Compute height recursively */
    int computeHeight(int idx) const;
};
