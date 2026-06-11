/**
 * @file AA12.h
 * @brief AA树(并发读写锁支持与基于时代的回收实现线程安全无锁读的平衡二叉搜索树) — AA Tree with Concurrent Read-Write Lock Support and Epoch-Based Reclamation for Thread-Safe Balanced BST with Lock-Free Reads
 *
 * 功能: 实现AA树(AA tree)，采用并发读写锁支持(concurrent read-write lock support)
 *       与基于时代的回收(epoch-based reclamation)实现线程安全无锁读的平衡二叉搜索树(thread-safe balanced BST with lock-free reads)。
 *
 * 协作: AVLTree(AVL树) / RedBlackTree(红黑树) / SkipList(跳表)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QReadWriteLock>

class AA12 : public QObject {
    Q_OBJECT

public:
    /** @brief Node in AA tree */
    struct Node {
        int key = 0;
        double value = 0.0;
        int level = 1;              // AA tree level (1 = leaf)
        int left = -1;              // Index into node pool (-1 = nil)
        int right = -1;
        int epoch = 0;              // Epoch when allocated (for reclamation)
        bool deleted = false;       // Tombstone for epoch-based reclamation
    };

    /** @brief Search result */
    struct SearchResult {
        bool found = false;
        double value = 0.0;
        double elapsedMs = 0.0;
    };

    /** @brief Range query result */
    struct RangeResult {
        QVector<QPair<int, double>> entries;
        int count = 0;
        double elapsedMs = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalInserts = 0;
        quint64 totalDeletes = 0;
        quint64 totalSearches = 0;
        int nodeCount = 0;
        int treeHeight = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit AA12(QObject *parent = nullptr);
    ~AA12() override;

    /** @brief Insert key-value pair (thread-safe) */
    bool insert(int key, double value);

    /** @brief Remove key (thread-safe, deferred via epoch) */
    bool remove(int key);

    /** @brief Search for key (lock-free read) */
    SearchResult search(int key) const;

    /** @brief Range query [lo, hi] */
    RangeResult rangeQuery(int lo, int hi) const;

    /** @brief Get all keys in sorted order */
    QVector<int> keys() const;

    /** @brief Advance epoch and reclaim nodes */
    void advanceEpoch();

    /** @brief Get current epoch */
    int currentEpoch() const { return m_epoch; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void insertDone(int key, int nodeCount, double timeMs);
    void removeDone(int key, bool success, double timeMs);

private:
    QVector<Node> m_nodes;
    int m_root = -1;
    int m_epoch = 0;
    mutable QReadWriteLock m_lock;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Skew operation (right rotation) */
    int skew(int nodeIdx);

    /** @brief Split operation (left rotation + level increase) */
    int split(int nodeIdx);

    /** @brief Recursive insert */
    int insertHelper(int nodeIdx, int key, double value);

    /** @brief Recursive delete */
    int removeHelper(int nodeIdx, int key);

    /** @brief Find minimum node in subtree */
    int findMin(int nodeIdx) const;

    /** @brief Recursive search */
    bool searchHelper(int nodeIdx, int key, double& value) const;

    /** @brief In-order traversal */
    void inOrder(int nodeIdx, QVector<QPair<int, double>>& result) const;

    /** @brief Range query helper */
    void rangeHelper(int nodeIdx, int lo, int hi, QVector<QPair<int, double>>& result) const;

    /** @brief Reclaim nodes from old epochs */
    void reclaimNodes();

    /** @brief Compute tree height */
    int height(int nodeIdx) const;
};
