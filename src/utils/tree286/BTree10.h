/**
 * @file BTree10.h
 * @brief B树(排序数据批量加载与前缀B树优化) — B-tree with Bulk-loading from Sorted Data and Prefix B-tree Optimization for Disk-efficient String Key Operations
 *
 * 功能: 实现B树(B-tree)，采用排序数据批量加载(bulk-loading from sorted data)
 *       与前缀B树优化(prefix B-tree optimization)实现磁盘高效字符串键操作(disk-efficient string key operations)。
 *
 * 协作: Treap15(树堆) / RedBlackTree11(红黑树) / BPlusTree9(B+树)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>

/**
 * @brief B树(排序数据批量加载与前缀B树优化)
 */
class BTree10 : public QObject {
    Q_OBJECT

public:
    /** @brief B-tree node */
    struct BTreeNode {
        QVector<int> keys;              // Integer keys
        QVector<QString> values;        // Associated string values
        QVector<int> children;          // Child node indices (-1 = leaf)
        int numKeys = 0;
        bool isLeaf = true;
        QString prefix;                 // Common prefix for string optimization
    };

    /** @brief Search result */
    struct SearchResult {
        bool found = false;
        int nodeIdx = -1;
        int keyIdx = -1;
        QString value;
        int depth = 0;
    };

    /** @brief Range query result */
    struct RangeResult {
        QVector<QPair<int, QString>> entries;
        int count = 0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numNodes = 0;
        int treeHeight = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit BTree10(int order = 4, QObject *parent = nullptr);
    ~BTree10() override;

    /** @brief Insert key-value pair */
    bool insert(int key, const QString& value);

    /** @brief Remove key */
    bool remove(int key);

    /** @brief Search by key */
    SearchResult search(int key) const;

    /** @brief Range query [lo, hi] */
    RangeResult rangeQuery(int lo, int hi) const;

    /** @brief Bulk-load from sorted key-value pairs */
    void bulkLoad(const QVector<QPair<int, QString>>& sortedData);

    /** @brief Optimize prefixes for string key nodes */
    void optimizePrefixes();

    int size() const { return m_size; }
    int order() const { return m_order; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void insertDone(int key, double timeMs);
    void removeDone(int key, bool success, double timeMs);

private:
    int m_order;             // Minimum degree t (max 2t-1 keys)
    int m_root = -1;
    int m_size = 0;
    Stats m_stats;
    double m_timeSum = 0.0;

    QVector<BTreeNode> m_nodes;
    QVector<int> m_freeList;

    /** @brief Allocate new node */
    int allocNode(bool isLeaf);

    /** @brief Free node */
    void freeNode(int idx);

    /** @brief Split full child node */
    void splitChild(int parentIdx, int childPos);

    /** @brief Insert non-full node */
    void insertNonFull(int nodeIdx, int key, const QString& value);

    /** @brief Remove from subtree rooted at nodeIdx */
    void removeFromNode(int nodeIdx, int key);

    /** @brief Borrow from left sibling */
    void borrowFromLeft(int nodeIdx, int childPos);

    /** @brief Borrow from right sibling */
    void borrowFromRight(int nodeIdx, int childPos);

    /** @brief Merge two children */
    void mergeChildren(int nodeIdx, int childPos);

    /** @brief Get predecessor key */
    QPair<int, QString> getPredecessor(int nodeIdx) const;

    /** @brief Get successor key */
    QPair<int, QString> getSuccessor(int nodeIdx) const;

    /** @brief Compute common prefix for node values */
    QString computeCommonPrefix(int nodeIdx) const;

    /** @brief Compute tree height */
    int computeHeight(int nodeIdx) const;

    /** @brief Range query helper */
    void rangeHelper(int nodeIdx, int lo, int hi,
                     RangeResult& result) const;
};
