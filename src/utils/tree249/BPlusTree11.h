/**
 * @file BPlusTree11.h
 * @brief B+树(后缀压缩内部节点+前缀查询优化的字符串键范围扫描) — B+ Tree with Suffix-Compressed Internal Nodes and Prefix Query Optimization for String Key Range Scans
 *
 * 功能: 实现B+树(B+ Tree)，使用后缀压缩内部节点(suffix-compressed
 *       internal nodes)减少存储开销，通过前缀查询优化(prefix query
 *       optimization)加速字符串键的范围扫描操作。
 *
 * 协作: AvlTree8(AVL树) / RBTree6(红黑树) / Trie10(字典树)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>

/**
 * @brief B+树(后缀压缩+前缀查询优化)
 */
class BPlusTree11 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numKeys = 0;
        int numNodes = 0;
        int treeHeight = 0;
        int numInsertions = 0;
        int numDeletions = 0;
        int numSearches = 0;
        int numSplits = 0;
        int numMerges = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit BPlusTree11(int order = 64, QObject *parent = nullptr);
    ~BPlusTree11() override;

    /** @brief Insert a string key with associated integer value */
    void insert(const QString& key, int value);

    /** @brief Remove a key from the tree */
    void remove(const QString& key);

    /** @brief Search for exact key match */
    int search(const QString& key) const;

    /** @brief Range scan from lowKey to highKey inclusive */
    QVector<QPair<QString, int>> rangeScan(const QString& lowKey,
                                            const QString& highKey) const;

    /** @brief Prefix search: all keys starting with prefix */
    QVector<QPair<QString, int>> prefixSearch(const QString& prefix) const;

    /** @brief Get number of stored keys */
    int size() const;

    /** @brief Clear the entire tree */
    void clear();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void treeModified(const QString& operation, int numKeys, int height, double timeMs);

private:
    int m_order;  // Maximum children per internal node

    /** @brief Tree node (internal or leaf) */
    struct Node {
        bool isLeaf = false;
        QVector<QString> keys;       // Suffix-compressed keys
        QVector<int> children;       // Child node indices (internal) or values (leaf)
        int parent = -1;
        int next = -1;               // Leaf linked list pointer
        QString prefix;              // Common prefix for this node
    };

    QVector<Node> m_nodes;
    int m_root = -1;
    int m_count = 0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Allocate a new node */
    int allocNode(bool isLeaf);

    /** @brief Find leaf node for a key */
    int findLeaf(const QString& key) const;

    /** @brief Split a leaf node */
    int splitLeaf(int nodeIdx);

    /** @brief Split an internal node */
    int splitInternal(int nodeIdx);

    /** @brief Insert into parent after child split */
    void insertIntoParent(int leftIdx, const QString& key, int rightIdx);

    /** @brief Compute common prefix length */
    int commonPrefixLen(const QString& a, const QString& b) const;

    /** @brief Compress keys using prefix extraction */
    void compressKeys(int nodeIdx);

    /** @brief Decompress key for comparison */
    QString decompressKey(int nodeIdx, int keyIdx) const;

    /** @brief Find prefix start leaf for prefix search */
    int findPrefixLeaf(const QString& prefix) const;

    /** @brief Merge or redistribute after deletion */
    void rebalanceAfterDelete(int nodeIdx);

    /** @brief Collect leaf values in range */
    void collectRange(int leafIdx, const QString& low, const QString& high,
                       QVector<QPair<QString, int>>& result) const;
};
