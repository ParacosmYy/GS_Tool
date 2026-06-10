/**
 * @file AA10.h
 * @brief AA树(递归skew/split重平衡与基于层级删除简化平衡树维护) — AA Tree with Recursive Skew/Split Rebalancing and Level-based Deletion for Simplified Balanced Tree Maintenance
 *
 * 功能: 实现AA树(AA tree)，采用递归skew/split重平衡(recursive skew/split rebalancing)
 *       与基于层级删除(level-based deletion)实现简化平衡树维护(simplified balanced tree maintenance)。
 *
 * 协作: AVLTree11(AVL树) / RedBlackTree12(红黑树) / SplayTree10(伸展树)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief AA树(递归skew/split重平衡与基于层级删除)
 */
class AA10 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numNodes = 0;
        int treeHeight = 0;
        int numInsertions = 0;
        int numDeletions = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit AA10(QObject *parent = nullptr);
    ~AA10() override;

    /** @brief Insert a key into the tree */
    void insert(int key);

    /** @brief Remove a key from the tree */
    void remove(int key);

    /** @brief Check if key exists in tree */
    bool contains(int key) const;

    /** @brief Search for key, returns true if found */
    bool search(int key) const;

    /** @brief In-order traversal, returns sorted keys */
    QVector<int> inOrder() const;

    /** @brief Get tree height (max level) */
    int height() const;

    /** @brief Get number of nodes */
    int size() const;

    /** @brief Clear all nodes */
    void clear();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void operationDone(const QString& op, int numNodes, double timeMs);

private:
    /** @brief AA tree node stored in pool */
    struct Node {
        int key = 0;
        int level = 1;
        int left = -1;     // Index in node pool, -1 = nil
        int right = -1;    // Index in node pool, -1 = nil
    };

    QVector<Node> m_nodes;     // Node pool
    int m_root = -1;           // Root index (-1 = empty)

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Skew: right-rotate to fix left horizontal link */
    int skew(int nodeIdx);

    /** @brief Split: left-rotate to fix consecutive right horizontal links */
    int split(int nodeIdx);

    /** @brief Recursive insert, returns new root of subtree */
    int insertRec(int nodeIdx, int key);

    /** @brief Recursive remove, returns new root of subtree */
    int removeRec(int nodeIdx, int key);

    /** @brief Find minimum key in subtree */
    int findMin(int nodeIdx) const;

    /** @brief Decrease level and rebalance after deletion */
    int decreaseLevel(int nodeIdx);

    /** @brief In-order traversal into result vector */
    void inOrderRec(int nodeIdx, QVector<int>& result) const;

    /** @brief Compute tree height from node levels */
    int computeHeight(int nodeIdx) const;

    /** @brief Allocate a new node, returns index */
    int allocNode(int key);
};
