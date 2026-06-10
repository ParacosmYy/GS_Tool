/**
 * @file BPlusTree13.h
 * @brief B+树(分数级联与缓冲批量更新的I/O高效范围查询处理) — B+ Tree with Fractional Cascading and Buffered Bulk Updates for I/O-efficient Range Query Processing
 *
 * 功能: 实现B+树(B+ tree)，采用分数级联(fractional cascading)
 *       与缓冲批量更新(buffered bulk updates)实现I/O高效范围查询处理(I/O-efficient range query processing)。
 *
 * 协作: AvlTree10(AVL树) / RedBlackTree12(红黑树) / BTree11(B树)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief B+树(分数级联与缓冲批量更新)
 */
class BPlusTree13 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numKeys = 0;
        int numNodes = 0;
        int treeHeight = 0;
        int numSplits = 0;
        int numMerges = 0;
        int numBulkUpdates = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit BPlusTree13(QObject *parent = nullptr);
    ~BPlusTree13() override;

    /** @brief Set order (max children per internal node) */
    void setOrder(int order);

    /** @brief Insert a key-value pair */
    void insert(int key, double value);

    /** @brief Remove a key */
    void remove(int key);

    /** @brief Find value by exact key match */
    double find(int key) const;

    /** @brief Range query using fractional cascading [lo, hi] */
    QVector<QPair<int, double>> rangeQuery(int lo, int hi) const;

    /** @brief Buffered bulk insert for I/O efficiency */
    void bulkInsert(const QVector<QPair<int, double>>& entries);

    /** @brief Flush internal buffer to tree */
    void flushBuffer();

    /** @brief Check if key exists */
    bool contains(int key) const;

    /** @brief Get all keys in sorted order */
    QVector<int> allKeys() const;

    /** @brief Get number of keys */
    int size() const;

    /** @brief Clear all data */
    void clear();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void operationDone(const QString& op, int numKeys, int treeHeight, double timeMs);

private:
    int m_order = 64;  // Max children per internal node

    /** @brief Tree node (serves as both internal and leaf) */
    struct Node {
        bool isLeaf = true;
        QVector<int> keys;              // Sorted keys
        QVector<double> values;         // Leaf: values; Internal: not used
        QVector<int> children;          // Internal: child node indices; Leaf: linked list
        int nextLeaf = -1;              // Leaf linked list pointer
        int parent = -1;                // Parent node index
        // Fractional cascading: auxiliary indices for fast range query
        QVector<int> cascadeIdx;        // Precomputed binary search hints
    };

    QVector<Node> m_nodes;
    int m_root = -1;

    // Buffer for bulk updates
    QVector<QPair<int, double>> m_buffer;
    int m_bufferSize = 256;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Allocate a new node, returns index */
    int allocNode(bool isLeaf);

    /** @brief Find leaf node that should contain key */
    int findLeaf(int key) const;

    /** @brief Insert into a leaf node */
    void insertIntoLeaf(int leafIdx, int key, double value);

    /** @brief Split a leaf node */
    void splitLeaf(int leafIdx);

    /** @brief Split an internal node */
    void splitInternal(int nodeIdx);

    /** @brief Insert key into parent after child split */
    void insertIntoParent(int leftIdx, int key, int rightIdx);

    /** @brief Remove from leaf, handle redistribution/merge */
    void removeFromLeaf(int leafIdx, int key);

    /** @brief Rebuild fractional cascading indices */
    void rebuildCascade(int leafIdx);

    /** @brief Rebuild cascade for all leaves */
    void rebuildAllCascade();

    /** @brief In-order traversal helper */
    void inOrderCollect(int nodeIdx, QVector<QPair<int, double>>& result) const;
};
