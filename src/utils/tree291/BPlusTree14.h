/**
 * @file BPlusTree14.h
 * @brief B+树(前缀压缩与批量加载实现磁盘高效范围查询与最小内部节点存储) — B+ Tree with Prefix Compression and Bulk Loading for Disk-efficient Range Queries with Minimal Internal Node Storage
 *
 * 功能: 实现B+树(B+ tree)，采用前缀压缩(prefix compression)
 *       与批量加载(bulk loading)实现磁盘高效范围查询(disk-efficient range queries)与最小内部节点存储(minimal internal node storage)。
 *
 * 协作: AvlTree11(AVL树) / RedBlackTree10(红黑树) / BTree12(B树)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>

/**
 * @brief B+树(前缀压缩与批量加载实现磁盘高效范围查询与最小内部节点存储)
 */
class BPlusTree14 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numKeys = 0;
        int numNodes = 0;
        int treeHeight = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit BPlusTree14(QObject *parent = nullptr);
    ~BPlusTree14() override;

    void setOrder(int order);   // Max children per internal node
    void setLeafCapacity(int cap);

    /** @brief Insert a key-value pair */
    void insert(int key, int value);

    /** @brief Remove a key */
    void remove(int key);

    /** @brief Search for exact key, returns value or -1 */
    int search(int key) const;

    /** @brief Range query: all key-value pairs in [lo, hi] */
    QVector<QPair<int,int>> rangeQuery(int lo, int hi) const;

    /** @brief Bulk load from sorted key-value pairs */
    void bulkLoad(const QVector<QPair<int,int>>& sortedPairs);

    int size() const { return m_size; }
    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void operationDone(const QString& op, int key, double timeMs);

private:
    static constexpr int NULL_IDX = -1;

    /** @brief Internal node stored in array pool */
    struct InternalNode {
        QVector<int> keys;             // Prefix-compressed separator keys
        QVector<int> children;         // Child node indices
        int parent = NULL_IDX;
        bool isLeaf = false;
    };

    /** @brief Leaf node with linked-list pointer */
    struct LeafNode {
        QVector<int> keys;
        QVector<int> values;
        int parent = NULL_IDX;
        int nextLeaf = NULL_IDX;       // Linked list for range scans
        bool isLeaf = true;
    };

    // Node pool: index < m_leafStart => internal, >= m_leafStart => leaf
    QVector<InternalNode> m_internalNodes;
    QVector<LeafNode> m_leafNodes;
    QVector<int> m_freeInternal;
    QVector<int> m_freeLeaf;

    int m_root = NULL_IDX;
    int m_order = 32;
    int m_leafCap = 32;
    int m_size = 0;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Allocate internal node */
    int allocInternal();

    /** @brief Allocate leaf node */
    int allocLeaf();

    /** @brief Find leaf node containing key */
    int findLeaf(int key) const;

    /** @brief Split leaf node */
    void splitLeaf(int leafIdx);

    /** @brief Split internal node */
    void splitInternal(int nodeIdx);

    /** @brief Insert separator into parent after child split */
    void insertIntoParent(int leftIdx, int key, int rightIdx);

    /** @brief Apply prefix compression to internal node keys */
    void compressPrefixes(int nodeIdx);

    /** @brief Rebalance after deletion (simplified borrow/merge) */
    void rebalanceLeaf(int leafIdx);
};
