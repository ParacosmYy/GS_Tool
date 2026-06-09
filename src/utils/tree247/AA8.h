/**
 * @file AA8.h
 * @brief AA树(层级平衡+删除后自底向上skew/split修复的简化红黑不变量) — AA Tree with Level-Based Balancing and Bottom-Up Skew/Split Fixup After Deletion for Simplified Red-Black Invariant
 *
 * 功能: 实现AA树(AA Tree)，基于层级(level)的平衡策略，通过skew和split
 *       操作维护简化红黑不变量(simplified red-black invariant)，删除后
 *       自底向上(bottom-up)执行skew/split修复。
 *
 * 协作: RBTree6(红黑树) / AVLTree5(AVL树) / SplayTree5(伸展树)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief AA树(层级平衡+删除后自底向上skew/split修复)
 */
class AA8 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numNodes = 0;
        int treeHeight = 0;
        int numInsertions = 0;
        int numDeletions = 0;
        int numSearches = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit AA8(QObject *parent = nullptr);
    ~AA8() override;

    /** @brief Insert a key */
    void insert(int key);

    /** @brief Remove a key */
    void remove(int key);

    /** @brief Search for a key, returns true if found */
    bool contains(int key) const;

    /** @brief In-order traversal (sorted order) */
    QVector<int> inorder() const;

    /** @brief Get number of nodes */
    int size() const;

    /** @brief Check if tree is empty */
    bool isEmpty() const;

    /** @brief Clear all nodes */
    void clear();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void treeModified(const QString& operation, int key, int numNodes, double timeMs);

private:
    /** @brief AA tree node */
    struct Node {
        int key = 0;
        int level = 1;
        int left = -1;   // Index into node pool, -1 = nil
        int right = -1;
    };

    QVector<Node> m_nodes;   // Node pool (index 0 is sentinel)
    int m_root = -1;
    int m_count = 0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Allocate a new node, return index */
    int allocNode(int key);

    /** @brief Skew: right rotation to fix left horizontal link */
    int skew(int nodeIdx);

    /** @brief Split: left rotation to fix consecutive right horizontal links */
    int split(int nodeIdx);

    /** @brief Recursive insert */
    int insertRec(int nodeIdx, int key);

    /** @brief Recursive remove */
    int removeRec(int nodeIdx, int key);

    /** @brief Find successor (leftmost in right subtree) */
    int successor(int nodeIdx) const;

    /** @brief Find predecessor (rightmost in left subtree) */
    int predecessor(int nodeIdx) const;

    /** @brief Decrease level and rebalance after deletion */
    int decreaseLevel(int nodeIdx);

    /** @brief In-order traversal helper */
    void inorderRec(int nodeIdx, QVector<int>& result) const;

    /** @brief Compute tree height */
    int computeHeight(int nodeIdx) const;
};
