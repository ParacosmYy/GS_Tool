/**
 * @file BPlusTree10.h
 * @brief B+树(有序输入批量加载+分数级联高效范围查询) — B+ Tree with Bulk-Loading from Sorted Input and Fractional Cascading for Efficient Range Queries
 *
 * 功能: 实现B+树(B+ tree)，支持从有序输入批量加载(bulk-loading)构建，并采用分数级联
 *       (fractional cascading)技术优化范围查询(range queries)效率，减少节点间搜索开销。
 *
 * 协作: AvlTree7(AVL树) / RedBlackTree6(红黑树) / BTree6(B树)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief B+树(有序输入批量加载+分数级联高效范围查询)
 */
class BPlusTree10 : public QObject {
    Q_OBJECT

public:
    /** @brief Key-value pair */
    using KVPair = QPair<int, double>;

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numKeys = 0;
        int treeHeight = 0;
        int numNodes = 0;
        int numRangeQueries = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit BPlusTree10(QObject *parent = nullptr);
    ~BPlusTree10() override;

    /** @brief Set order (max keys per internal node) */
    void setOrder(int order);

    /** @brief Bulk-load from sorted key-value pairs */
    void bulkLoad(const QVector<KVPair>& sortedPairs);

    /** @brief Insert a single key-value pair */
    void insert(int key, double value);

    /** @brief Remove a key */
    void remove(int key);

    /** @brief Search for key, returns value (NaN if not found) */
    double search(int key) const;

    /** @brief Range query: all pairs in [lo, hi] using fractional cascading */
    QVector<KVPair> rangeQuery(int lo, int hi) const;

    /** @brief Check if key exists */
    bool contains(int key) const;

    /** @brief Get all key-value pairs (in-order) */
    QVector<KVPair> allPairs() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void bulkLoadCompleted(int numKeys, int height, double timeMs);
    void rangeQueryCompleted(int lo, int hi, int count);
    void keyInserted(int key);
    void keyRemoved(int key);

private:
    /** @brief Internal node */
    struct InternalNode {
        QVector<int> keys;
        QVector<void*> children;
        // Fractional cascading: augmented indices for efficient search
        QVector<int> cascadeIndices;
    };

    /** @brief Leaf node */
    struct LeafNode {
        QVector<int> keys;
        QVector<double> values;
        LeafNode* next = nullptr;  // Linked list for range scan
        // Fractional cascading: sample of parent separator keys
        QVector<int> bridgeKeys;
        QVector<int> bridgeIndices;
    };

    int m_order = 32;
    void* m_root = nullptr;
    bool m_rootIsLeaf = true;
    LeafNode* m_firstLeaf = nullptr;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Find leaf node containing key */
    LeafNode* findLeaf(int key) const;

    /** @brief Binary search within leaf for key index */
    int leafSearch(LeafNode* leaf, int key) const;

    /** @brief Binary search with fractional cascade hint */
    int cascadedSearch(LeafNode* leaf, int key, int hint) const;

    /** @brief Build bridge tables for fractional cascading */
    void buildBridges(InternalNode* parent, int childIdx);

    /** @brief Split leaf node */
    LeafNode* splitLeaf(LeafNode* leaf);

    /** @brief Insert into internal node after child split */
    void insertIntoInternal(InternalNode* node, int key, void* rightChild);

    /** @brief Delete all nodes */
    void deleteTree();

    /** @brief Collect all pairs via leaf scan */
    void collectPairs(LeafNode* leaf, QVector<KVPair>& result) const;
};
