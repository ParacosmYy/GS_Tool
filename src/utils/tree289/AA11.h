/**
 * @file AA11.h
 * @brief AA树(自底向上再平衡与基于层级的倾斜-分裂不变量简化平衡BST维护) — AA Tree with Bottom-up Rebalancing and Level-based Skew-split Invariants for Simplified Balanced BST Maintenance
 *
 * 功能: 实现AA树(AA tree)，采用自底向上再平衡(bottom-up rebalancing)
 *       与基于层级的倾斜-分裂不变量(level-based skew-split invariants)实现简化平衡BST维护(simplified balanced BST maintenance)。
 *
 * 协作: AVLTree9(AVL树) / RedBlackTree10(红黑树) / SplayTree8(伸展树)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>

/**
 * @brief AA树(自底向上再平衡与基于层级的倾斜-分裂不变量简化平衡BST维护)
 */
class AA11 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numNodes = 0;
        int treeHeight = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit AA11(QObject *parent = nullptr);
    ~AA11() override;

    /** @brief Insert a key-value pair */
    void insert(int key, int value);

    /** @brief Remove a key */
    void remove(int key);

    /** @brief Search for a key, returns value or -1 */
    int search(int key) const;

    /** @brief Check if key exists */
    bool contains(int key) const;

    /** @brief In-order traversal: collect all key-value pairs */
    QVector<QPair<int,int>> toVector() const;

    /** @brief Get all keys in sorted order */
    QVector<int> keys() const;

    int size() const { return m_size; }
    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void operationDone(const QString& op, int key, double timeMs);

private:
    static constexpr int NULL_NODE = -1;

    /** @brief AA tree node stored in vector */
    struct AANode {
        int key = 0;
        int value = 0;
        int level = 1;              // AA tree level (leaf = 1)
        int left = NULL_NODE;
        int right = NULL_NODE;
    };

    QVector<AANode> m_nodes;
    QVector<int> m_freeList;
    int m_root = NULL_NODE;
    int m_size = 0;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Allocate a new node */
    int allocNode(int key, int value);

    /** @brief Free a node to pool */
    void freeNode(int idx);

    /** @brief Skew: right rotation to fix left-horizontal link */
    int skew(int node);

    /** @brief Split: left rotation to fix consecutive right-horizontal links */
    int split(int node);

    /** @brief Recursive insert */
    int insertRec(int node, int key, int value, bool& inserted);

    /** @brief Recursive remove */
    int removeRec(int node, int key, bool& removed);

    /** @brief Find minimum node in subtree */
    int findMin(int node) const;

    /** @brief In-order traversal helper */
    void inOrder(int idx, QVector<QPair<int,int>>& result) const;

    /** @brief Compute tree height */
    int computeHeight(int idx) const;
};
