/**
 * @file AA9.h
 * @brief AA树(自顶向下删除修复层级平衡简化平衡树操作) — AA Tree with Top-down Deletion Fixup and Level-based Rebalancing for Simplified Balanced Tree Operations
 *
 * 功能: 实现AA树(AA tree)数据结构，采用自顶向下删除修复(top-down
 *       deletion fixup)和基于层级的平衡(level-based rebalancing)
 *       简化平衡树操作，提供高效的有序集合管理。
 *
 * 协作: AVLTree8(AVL树) / RedBlackTree9(红黑树) / BTree8(B树)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief AA树(自顶向下删除修复层级平衡简化平衡树操作)
 */
class AA9 : public QObject {
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

    explicit AA9(QObject *parent = nullptr);
    ~AA9() override;

    /** @brief Insert a key */
    void insert(int key);

    /** @brief Remove a key */
    void remove(int key);

    /** @brief Check if key exists */
    bool contains(int key) const;

    /** @brief In-order traversal */
    QVector<int> inOrder() const;

    /** @brief Get tree height */
    int height() const;

    /** @brief Get node count */
    int size() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void treeModified(int numNodes, int height, double timeMs);

private:
    /** @brief AA tree node */
    struct Node {
        int key = 0;
        int level = 1;
        Node* left = nullptr;
        Node* right = nullptr;
    };

    Node* m_root = nullptr;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Skew rotation (right rotation to fix left-horizontal link) */
    Node* skew(Node* node);

    /** @brief Split operation (left rotation to fix double right-horizontal links) */
    Node* split(Node* node);

    /** @brief Recursive insert */
    Node* insertHelper(Node* node, int key);

    /** @brief Top-down delete with fixup */
    Node* removeHelper(Node* node, int key);

    /** @brief Find minimum in subtree */
    Node* findMin(Node* node) const;

    /** @brief Decrease level and rebalance */
    Node* decreaseLevel(Node* node);

    /** @brief Recursive in-order */
    void inOrderHelper(Node* node, QVector<int>& result) const;

    /** @brief Compute height recursively */
    int heightHelper(Node* node) const;

    /** @brief Delete all nodes */
    void clearHelper(Node* node);
};
