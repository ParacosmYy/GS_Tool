/**
 * @file BTree5.h
 * @brief B树(字符串键前缀压缩+批量加载兄弟指针顺序扫描) — B-Tree with Prefix Compression for String Keys and Bulk-Loaded Sibling Pointers for Sequential Scan
 *
 * 功能: 实现B树数据结构，支持字符串键前缀压缩以减少内存占用，
 *       批量加载时建立兄弟指针支持高效顺序扫描。
 *
 * 协作: Rope7(绳索) / RadixTree9(基数树) / LSMTree6(LSM树)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>

/**
 * @brief B树(前缀压缩+兄弟指针顺序扫描)
 */
class BTree5 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numKeys = 0;
        int numNodes = 0;
        int treeHeight = 0;
        int order = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit BTree5(QObject *parent = nullptr);
    ~BTree5() override;

    /** @brief Set B-tree order (min degree) */
    void setParameters(int order = 64);

    /** @brief Insert key-value pair */
    void insert(const QString& key, int value);

    /** @brief Search for key, return value (-1 if not found) */
    int search(const QString& key) const;

    /** @brief Remove key */
    bool remove(const QString& key);

    /** @brief Bulk load from sorted key-value pairs */
    void bulkLoad(const QVector<QPair<QString, int>>& sortedPairs);

    /** @brief Sequential scan using sibling pointers */
    QVector<QPair<QString, int>> sequentialScan() const;

    /** @brief Range query [loKey, hiKey] */
    QVector<QPair<QString, int>> rangeQuery(const QString& lo,
                                             const QString& hi) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void operationCompleted(const QString& op, int numKeys, double timeMs);

private:
    int m_order = 64;    // Min degree
    int m_root = -1;

    // Node stored in pool
    struct Node {
        bool isLeaf = true;
        QVector<QString> keys;         // Prefix-compressed keys
        QVector<QString> prefixes;     // Decompressed full keys for split points
        QVector<int> values;           // Leaf only
        QVector<int> children;         // Internal only
        int next = -1;                 // Sibling pointer for sequential scan
        int parent = -1;
    };

    QVector<Node> m_nodes;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Allocate new node */
    int allocNode(bool isLeaf = true);

    /** @brief Compute common prefix of two strings */
    QString commonPrefix(const QString& a, const QString& b) const;

    /** @brief Compress key by removing prefix */
    QString compressKey(const QString& full, const QString& prefix) const;

    /** @brief Split full child node */
    void splitChild(int parentIdx, int childIdx);

    /** @brief Insert into non-full node */
    void insertNonFull(int nodeIdx, const QString& key, int value);

    /** @brief Find leaf node for key */
    int findLeaf(const QString& key) const;

    /** @brief Link sibling pointers during bulk load */
    void linkSiblings();

    /** @brief Collect all key-value pairs from subtree */
    void collectSubtree(int nodeIdx, QVector<QPair<QString, int>>& result) const;
};
