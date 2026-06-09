/**
 * @file AvlTree8.h
 * @brief AVL树(并发读优化的写时复制节点+细粒度轮转回收机制) — AVL Tree with Concurrent Read Optimization Using Copy-on-Write Nodes and Fine-Grained Epoch-Based Reclamation
 *
 * 功能: 实现AVL树(AVL Tree)，使用写时复制节点(copy-on-write nodes)实现
 *       无锁并发读(lock-free concurrent reads)，通过细粒度轮转回收
 *       (fine-grained epoch-based reclamation)安全回收旧节点。
 *
 * 协作: RBTree6(红黑树) / AA8(AA树) / SplayTree5(伸展树)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QAtomicInt>

/**
 * @brief AVL树(写时复制并发读+轮转回收)
 */
class AvlTree8 : public QObject {
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
        int numCopies = 0;
        int epoch = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit AvlTree8(QObject *parent = nullptr);
    ~AvlTree8() override;

    /** @brief Insert a key-value pair */
    void insert(int key, int value = 0);

    /** @brief Remove a key */
    void remove(int key);

    /** @brief Search for a key, returns value or -1 */
    int search(int key) const;

    /** @brief Check if key exists */
    bool contains(int key) const;

    /** @brief In-order traversal (snapshot for concurrent safety) */
    QVector<QPair<int, int>> inorder() const;

    /** @brief Get number of nodes */
    int size() const;

    /** @brief Clear tree and reclaim all nodes */
    void clear();

    /** @brief Advance epoch and reclaim old nodes */
    void advanceEpoch();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void treeModified(const QString& operation, int key, int numNodes, double timeMs);

private:
    /** @brief Immutable tree node (copy-on-write) */
    struct Node {
        int key;
        int value;
        int height;
        int left = -1;   // Index into node pool
        int right = -1;
        int refCount = 1;
        int birthEpoch = 0;
    };

    QVector<Node> m_pool;        // Node pool
    QVector<int> m_freeList;     // Free node indices
    int m_root = -1;
    int m_count = 0;

    QAtomicInt m_epoch;
    QVector<int> m_retired[3];   // 3 epoch buckets for reclamation

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Allocate a new node (copy-on-write) */
    int allocNode(int key, int value, int epoch);

    /** @brief Clone a node for COW modification */
    int cloneNode(int nodeIdx, int epoch);

    /** @brief Retire a node to epoch-based reclamation */
    void retireNode(int nodeIdx);

    /** @brief Reclaim nodes from oldest epoch */
    void reclaimEpoch(int oldEpoch);

    /** @brief Get node height (0 for null) */
    int getHeight(int nodeIdx) const;

    /** @brief Update node height from children */
    void updateHeight(int nodeIdx);

    /** @brief Balance factor */
    int balanceFactor(int nodeIdx) const;

    /** @brief Rotate right, returns new root index */
    int rotateRight(int nodeIdx);

    /** @brief Rotate left, returns new root index */
    int rotateLeft(int nodeIdx);

    /** @brief Balance node */
    int balance(int nodeIdx);

    /** @brief Recursive insert with COW */
    int insertRec(int nodeIdx, int key, int value, int epoch);

    /** @brief Recursive remove with COW */
    int removeRec(int nodeIdx, int key, int epoch);

    /** @brief Find min node */
    int findMin(int nodeIdx) const;

    /** @brief In-order traversal helper */
    void inorderRec(int nodeIdx, QVector<QPair<int, int>>& result) const;
};
