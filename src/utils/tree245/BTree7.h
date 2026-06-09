/**
 * @file BTree7.h
 * @brief B树(批量构建+前缀压缩键的空间高效磁盘友好树) — B-Tree with Bulk-Loading and Prefix-Compressed Keys for Space-Efficient Disk-Friendly Tree Operations
 *
 * 功能: 实现B树(B-Tree)数据结构，支持批量构建(bulk-loading)从有序数据高效
 *       建树，使用前缀压缩键(prefix-compressed keys)减少内部节点空间占用，
 *       提供磁盘友好的有序映射操作(disk-friendly tree operations)。
 *
 * 协作: BPlusTree7(B+树) / Rope9(绳索) / SplayTree5(伸展树)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>

/**
 * @brief B树(批量构建+前缀压缩键的空间高效磁盘友好树)
 */
class BTree7 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numNodes = 0;
        int treeHeight = 0;
        int numKeys = 0;
        int order = 0;
        int numSplits = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit BTree7(QObject *parent = nullptr);
    ~BTree7() override;

    /** @brief Set B-tree order (min degree t, max 2t-1 keys per node) */
    void setOrder(int t);

    /** @brief Bulk-load from pre-sorted key-value pairs */
    void bulkLoad(const QVector<QPair<QString, double>>& sortedPairs);

    /** @brief Insert a key-value pair */
    void insert(const QString& key, double value);

    /** @brief Search for a key, returns value or NaN if not found */
    double search(const QString& key) const;

    /** @brief Remove a key */
    bool remove(const QString& key);

    /** @brief Check if key exists */
    bool contains(const QString& key) const;

    /** @brief Get all keys in sorted order */
    QVector<QString> keys() const;

    /** @brief Get all values in sorted key order */
    QVector<double> values() const;

    /** @brief Get number of stored keys */
    int size() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void operationCompleted(const QString& op, int numKeys, int height, double timeMs);

private:
    int m_order = 50;  // Minimum degree t

    /** @brief B-tree node stored in arena */
    struct Node {
        QVector<QString> keys;
        QVector<double> vals;
        QVector<int> children;
        int parent = -1;
        bool isLeaf = true;
        // Prefix compression metadata
        QString commonPrefix;    // Shared prefix of all keys in this node
        QVector<int> suffixLen;  // Length of suffix after removing commonPrefix
    };

    QVector<Node> m_nodes;
    int m_root = -1;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Allocate new node */
    int allocNode();

    /** @brief Split full child node */
    void splitChild(int parentIdx, int childPos);

    /** @brief Insert into non-full node */
    void insertNonFull(int nodeIdx, const QString& key, double value);

    /** @brief Search recursively */
    double searchRec(int nodeIdx, const QString& key) const;

    /** @brief Remove recursively */
    bool removeRec(int nodeIdx, const QString& key);

    /** @brief Find predecessor key in subtree */
    QPair<QString, double> findMax(int nodeIdx) const;

    /** @brief Find successor key in subtree */
    QPair<QString, double> findMin(int nodeIdx) const;

    /** @brief Update prefix compression for a node */
    void updatePrefix(int nodeIdx);

    /** @brief Extract full key from compressed representation */
    QString fullKey(int nodeIdx, int keyIdx) const;

    /** @brief Collect keys in order */
    void collectKeys(int nodeIdx, QVector<QString>& result) const;

    /** @brief Collect values in order */
    void collectValues(int nodeIdx, QVector<double>& result) const;

    /** @brief String comparison helper */
    static int compareKeys(const QString& a, const QString& b);
};
