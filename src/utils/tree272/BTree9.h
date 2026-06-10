/**
 * @file BTree9.h
 * @brief B树(批量加载与前缀压缩键缓存友好外存搜索) — B-Tree with Bulk-Loading and Prefix-Compressed Keys for Cache-Friendly External Memory Search Operations
 *
 * 功能: 实现B树(B-tree)，采用批量加载(bulk-loading)
 *       与前缀压缩键(prefix-compressed keys)实现缓存友好外存搜索(cache-friendly external memory search)。
 *
 * 协作: Treap14(树堆) / SplayTree12(伸展树) / WeightBalancedTree10(权重平衡树)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>

/**
 * @brief B树(批量加载与前缀压缩键缓存友好外存搜索)
 */
class BTree9 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numNodes = 0;
        int treeHeight = 0;
        int numKeys = 0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Key-value entry */
    struct Entry {
        QString key;
        int value = 0;
    };

    explicit BTree9(QObject *parent = nullptr);
    ~BTree9() override;

    /** @brief Set minimum degree (order) of B-tree */
    void setOrder(int t);

    /** @brief Insert a key-value pair */
    void insert(const QString& key, int value);

    /** @brief Search for key, returns value (-1 if not found) */
    int search(const QString& key) const;

    /** @brief Remove a key, returns true if found */
    bool remove(const QString& key);

    /** @brief Bulk-load from sorted entries (builds balanced tree bottom-up) */
    void bulkLoad(const QVector<Entry>& sortedEntries);

    /** @brief Get all keys in sorted order */
    QVector<QString> inOrderKeys() const;

    /** @brief Get tree height */
    int height() const;

    /** @brief Get total key count */
    int size() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void treeUpdated(int numKeys, int height, double timeMs);

private:
    int m_order = 64;   // minimum degree

    /** @brief B-tree node stored in pool */
    struct BNode {
        QVector<QString> keys;      // prefix-compressed keys
        QVector<int> values;
        QVector<int> children;      // child node indices (-1 = leaf)
        bool isLeaf = true;
        int prefixLen = 0;          // common prefix length for this node
    };

    int m_root = -1;
    QVector<BNode> m_nodes;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Allocate a new node */
    int allocNode(bool isLeaf);

    /** @brief Compute common prefix length of two strings */
    int commonPrefix(const QString& a, const QString& b) const;

    /** @brief Compress keys in a node using prefix compression */
    void compressPrefix(BNode& node) const;

    /** @brief Decompress a key at given index in node */
    QString decompressKey(const BNode& node, int idx) const;

    /** @brief Split full child node */
    void splitChild(int parentIdx, int childPos);

    /** @brief Insert into non-full node */
    void insertNonFull(int nodeIdx, const QString& key, int value);

    /** @brief Search helper */
    int searchHelper(int nodeIdx, const QString& key) const;

    /** @brief In-order traversal helper */
    void inOrderHelper(int nodeIdx, QVector<QString>& result) const;

    /** @brief Height helper */
    int heightHelper(int nodeIdx) const;

    /** @brief Find key index in node using binary search with prefix decompression */
    int findKeyIndex(const BNode& node, const QString& key) const;
};
