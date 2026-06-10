/**
 * @file AvlTree10.h
 * @brief AVL树(基于排名的线索遍历与批量插入重平衡摊还O(log n)批量操作) — AVL Tree with Rank-based Threaded Traversal and Bulk-insertion Rebalancing for Amortized O(log n) Batch Operations
 *
 * 功能: 实现AVL树(AVL tree)，采用基于排名的线索遍历(rank-based threaded traversal)
 *       与批量插入重平衡(bulk-insertion rebalancing)实现摊还O(log n)批量操作
 *       (amortized O(log n) batch operations)。
 *
 * 协作: RedBlackTree12(红黑树) / SplayTree10(伸展树) / BTree11(B树)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief AVL树(基于排名的线索遍历与批量插入重平衡)
 */
class AvlTree10 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numNodes = 0;
        int treeHeight = 0;
        int numRotations = 0;
        int numBulkInserts = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit AvlTree10(QObject *parent = nullptr);
    ~AvlTree10() override;

    /** @brief Insert a single key */
    void insert(int key);

    /** @brief Bulk insert sorted keys for amortized O(log n) */
    void bulkInsert(const QVector<int>& sortedKeys);

    /** @brief Remove a key */
    void remove(int key);

    /** @brief Check if key exists */
    bool contains(int key) const;

    /** @brief Select k-th smallest element by rank (0-indexed) */
    int selectByRank(int k) const;

    /** @brief Get rank of key (number of elements < key) */
    int rank(int key) const;

    /** @brief In-order traversal using threaded links */
    QVector<int> threadedInOrder() const;

    /** @brief Get tree height */
    int height() const;

    /** @brief Get number of nodes */
    int size() const;

    /** @brief Clear all nodes */
    void clear();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void operationDone(const QString& op, int numNodes, int rotations, double timeMs);

private:
    /** @brief AVL node with rank and thread fields */
    struct Node {
        int key = 0;
        int height = 1;     // Subtree height
        int subtreeSize = 1; // Rank: size of left + right + 1
        int left = -1;       // Index in pool, -1 = nil
        int right = -1;
        int thread = -1;     // Thread pointer for in-order successor
        bool isThreadRight = false; // true: right is thread, not child
    };

    QVector<Node> m_nodes;
    int m_root = -1;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Allocate node, returns index */
    int allocNode(int key);

    /** @brief Get node height (0 for nil) */
    int nodeHeight(int idx) const;

    /** @brief Get node subtree size (0 for nil) */
    int nodeSize(int idx) const;

    /** @brief Update height and size from children */
    void updateNode(int idx);

    /** @brief Balance factor */
    int balanceFactor(int idx) const;

    /** @brief Right rotation, returns new root index */
    int rotateRight(int idx);

    /** @brief Left rotation, returns new root index */
    int rotateLeft(int idx);

    /** @brief Rebalance node after insertion/deletion */
    int rebalance(int idx);

    /** @brief Recursive insert, returns new subtree root */
    int insertRec(int idx, int key);

    /** @brief Recursive remove, returns new subtree root */
    int removeRec(int idx, int key);

    /** @brief Find minimum key in subtree */
    int findMin(int idx) const;

    /** @brief Build balanced tree from sorted keys (bulk insert) */
    int buildBalanced(const QVector<int>& keys, int start, int end);

    /** @brief Rebuild threads for in-order traversal */
    void rebuildThreads();
    void rebuildThreadsRec(int idx, int& prev);
};
