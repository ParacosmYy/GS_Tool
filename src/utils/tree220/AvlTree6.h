/**
 * @file AvlTree6.h
 * @brief AVL树(写时复制快照并发读+世代内存回收) — AVL Tree with Concurrent Read Support via Copy-on-Write Snapshot and Epoch-Based Memory Reclamation
 *
 * 功能: 实现AVL平衡二叉搜索树，支持写时复制快照实现无锁并发读，
 *       通过世代(epoch)机制安全回收旧版本内存。
 *
 * 协作: AA6(AA树) / RedBlackTree5(红黑树) / SplayTree4(伸展树)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QAtomicInt>

/**
 * @brief AVL树(写时复制快照+世代内存回收)
 */
class AvlTree6 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int nodeCount = 0;
        int treeHeight = 0;
        int numSnapshots = 0;
        int numInserts = 0;
        int numDeletes = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit AvlTree6(QObject *parent = nullptr);
    ~AvlTree6() override;

    /** @brief Insert key-value pair */
    void insert(int key, double value);

    /** @brief Remove key */
    void remove(int key);

    /** @brief Search for key, returns value or NaN */
    double search(int key) const;

    /** @brief Check if key exists */
    bool contains(int key) const;

    /** @brief Create a copy-on-write snapshot for concurrent reads */
    void createSnapshot();

    /** @brief Reclaim memory from old epochs */
    void reclaimEpoch();

    /** @brief In-order traversal keys */
    QVector<int> inOrderKeys() const;

    /** @brief Clear all nodes */
    void clear();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void operationCompleted(const QString& op, int key, double timeMs);
    void snapshotCreated(int snapshotId, int nodeCount);

private:
    struct Node {
        int key = 0;
        double value = 0.0;
        int height = 1;
        int refCount = 0;   // Reference count for COW
        Node* left = nullptr;
        Node* right = nullptr;
    };

    Node* m_root = nullptr;
    QAtomicInt m_currentEpoch;

    // Snapshot storage: old roots waiting for epoch reclamation
    QVector<Node*> m_snapshots;
    QVector<int> m_snapshotEpochs;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Get node height */
    int getHeight(Node* node) const;

    /** @brief Get balance factor */
    int balanceFactor(Node* node) const;

    /** @brief Update node height */
    void updateHeight(Node* node);

    /** @brief Right rotation */
    Node* rotateRight(Node* y);

    /** @brief Left rotation */
    Node* rotateLeft(Node* x);

    /** @brief Balance node via rotations */
    Node* balance(Node* node);

    /** @brief Copy-on-write clone of node */
    Node* cowClone(Node* node);

    /** @brief Recursive insert with COW */
    Node* insertImpl(Node* node, int key, double value);

    /** @brief Find minimum node */
    Node* findMin(Node* node) const;

    /** @brief Recursive remove with COW */
    Node* removeImpl(Node* node, int key);

    /** @brief Recursive search */
    double searchImpl(Node* node, int key) const;

    /** @brief In-order traversal */
    void inOrderImpl(Node* node, QVector<int>& result) const;

    /** @brief Recursive delete with ref counting */
    void safeDelete(Node* node);

    /** @brief Compute tree height */
    int computeHeight(Node* node) const;
};
