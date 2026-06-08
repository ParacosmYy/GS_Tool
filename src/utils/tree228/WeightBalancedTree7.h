/**
 * @file WeightBalancedTree7.h
 * @brief 权平衡树(松弛平衡准则+并发更新局部旋转) — Weight-Balanced Tree with Relaxed Balance Criteria and Concurrent Update via Local Rebalancing Rotations
 *
 * 功能: 实现权平衡树(WBB-tree)，使用松弛平衡准则(relaxed balance)降低
 *       重新平衡频率，支持并发更新(concurrent update)的局部旋转重平衡。
 *
 * 协作: AVLTree5(AVL树) / RedBlackTree6(红黑树) / BPlusTree7(B+树)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 权平衡树(松弛平衡+并发局部旋转)
 */
class WeightBalancedTree7 : public QObject {
    Q_OBJECT

public:
    /** @brief Tree node */
    struct Node {
        int key = 0;
        double value = 0.0;
        int weight = 1;           // subtree size
        int height = 1;
        int left = -1;
        int right = -1;
        int parent = -1;
        bool needsRebalance = false; // relaxed balance flag
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numNodes = 0;
        int treeHeight = 0;
        int numRotations = 0;
        int numInsertions = 0;
        int numDeletions = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit WeightBalancedTree7(QObject *parent = nullptr);
    ~WeightBalancedTree7() override;

    /** @brief Set balance parameter alpha (0.25..0.5) */
    void setAlpha(double alpha);

    /** @brief Insert key-value pair */
    bool insert(int key, double value);

    /** @brief Remove key */
    bool remove(int key);

    /** @brief Search for key, return value (-1 if not found) */
    double search(int key) const;

    /** @brief Get all keys in sorted order */
    QVector<int> inorderKeys() const;

    /** @brief Force rebalance of all flagged nodes */
    void rebalanceAll();

    int size() const { return m_stats.numNodes; }
    int height() const;
    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void treeUpdated(int nodes, int height, double timeMs);

private:
    double m_alpha = 0.29;    // balance threshold (weight ratio)
    int m_maxRebalanceBatch = 32;

    QVector<Node> m_nodes;
    int m_root = -1;
    int m_freeList = -1;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Allocate a new node */
    int allocateNode(int key, double value);

    /** @brief Free a node to free list */
    void freeNode(int idx);

    /** @brief Update weight and height from children */
    void updateAggregate(int idx);

    /** @brief Check if subtree rooted at idx is alpha-balanced */
    bool isBalanced(int idx) const;

    /** @brief Perform local rebalancing rotations */
    int rebalance(int idx);

    /** @brief Left rotation */
    int rotateLeft(int idx);

    /** @brief Right rotation */
    int rotateRight(int idx);

    /** @brief Rebuild subtree to perfect balance */
    int rebuildSubtree(int idx);

    /** @brief Collect subtree nodes in sorted order */
    void collectInorder(int idx, QVector<int>& keys,
                        QVector<double>& vals) const;

    /** @brief Build balanced subtree from sorted arrays */
    int buildBalanced(const QVector<int>& keys,
                      const QVector<double>& vals,
                      int lo, int hi);
};
