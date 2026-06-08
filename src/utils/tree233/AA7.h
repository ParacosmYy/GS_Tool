/**
 * @file AA7.h
 * @brief AA树(自顶向下skew-split删除+基于层级的epoch并发读垃圾回收) — AA Tree with Top-Down Skew-Split Deletion and Level-Based Concurrent Read with Epoch-Based Garbage Collection
 *
 * 功能: 实现AA树(AA tree)平衡二叉搜索树，采用自顶向下skew-split删除(top-down skew-split
 *       deletion)算法，支持基于层级(level-based)的epoch并发读(concurrent read)与epoch-based
 *       垃圾回收(garbage collection)。
 *
 * 协作: AVLTree5(AVL树) / RedBlackTree6(红黑树) / BTree6(B树)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QAtomicInt>

/**
 * @brief AA树(自顶向下skew-split删除+基于层级的epoch并发读垃圾回收)
 */
class AA7 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numNodes = 0;
        int treeHeight = 0;
        int numSkews = 0;
        int numSplits = 0;
        int numGCycles = 0;
        int numNodesReclaimed = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit AA7(QObject *parent = nullptr);
    ~AA7() override;

    /** @brief Insert key-value pair */
    void insert(int key, double value);

    /** @brief Remove key */
    void remove(int key);

    /** @brief Search for key, returns value (NaN if not found) */
    double search(int key) const;

    /** @brief Check if key exists */
    bool contains(int key) const;

    /** @brief In-order traversal */
    QVector<QPair<int, double>> inorderTraversal() const;

    /** @brief Trigger epoch-based garbage collection */
    void collectGarbage();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void nodeInserted(int key);
    void nodeRemoved(int key);
    void garbageCollected(int reclaimedNodes);

private:
    /** @brief AA tree node */
    struct Node {
        int key = 0;
        double value = 0.0;
        int level = 1;
        Node* left = nullptr;
        Node* right = nullptr;
        int epoch = 0;          // epoch when removed
        bool marked = false;    // marked for GC
    };

    Node* m_root = nullptr;

    // Epoch-based GC
    QAtomicInt m_currentEpoch;
    QVector<Node*> m_retiredNodes;     // nodes pending reclamation
    int m_gcThreshold = 64;            // trigger GC when retired count exceeds this

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Skew: right rotation to fix left-horizontal link */
    Node* skew(Node* node);

    /** @brief Split: left rotation to fix consecutive right-horizontal links */
    Node* split(Node* node);

    /** @brief Recursive insert */
    Node* insertNode(Node* node, int key, double value);

    /** @brief Top-down skew-split deletion */
    Node* removeNode(Node* node, int key);

    /** @brief Find minimum in subtree */
    Node* findMin(Node* node) const;

    /** @brief Recursive search */
    double searchNode(Node* node, int key) const;

    /** @brief In-order traversal helper */
    void inorderHelper(Node* node, QVector<QPair<int, double>>& result) const;

    /** @brief Delete entire subtree */
    void deleteTree(Node* node);

    /** @brief Compute tree height */
    int computeHeight(Node* node) const;

    /** @brief Retire node for epoch-based GC */
    void retireNode(Node* node);
};
