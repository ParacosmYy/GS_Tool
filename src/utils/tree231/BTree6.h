/**
 * @file BTree6.h
 * @brief B树(有序批量加载+缓冲树延迟刷写批量插入) — B-tree with Bulk Loading from Sorted Runs and Buffer-tree Lazy Flushing for Efficient Batch Insertions
 *
 * 功能: 实现B树数据结构，支持从有序序列批量加载(bulk loading)构建初始树，
 *       使用缓冲树(buffer-tree)机制实现延迟刷写(lazy flushing)以高效处理批量插入。
 *
 * 协作: BPlusTree3(B+树) / Rope8(绳索) / AVLTree5(AVL树)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief B树(有序批量加载+缓冲树延迟刷写)
 */
class BTree6 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numKeys = 0;
        int numNodes = 0;
        int treeHeight = 0;
        int bufferSize = 0;
        int numFlushes = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit BTree6(QObject *parent = nullptr);
    ~BTree6() override;

    /** @brief Set minimum degree (order) and buffer capacity */
    void setParameters(int minDegree, int bufferCapacity = 256);

    /** @brief Bulk load from sorted key-value pairs for efficient initial build */
    bool bulkLoad(const QVector<double>& sortedKeys,
                  const QVector<double>& sortedValues);

    /** @brief Insert a single key-value pair (may buffer) */
    void insert(double key, double value);

    /** @brief Search for a key, return value (-1 if not found) */
    double search(double key) const;

    /** @brief Remove a key */
    bool remove(double key);

    /** @brief Range query: all key-value pairs in [lo, hi] */
    QVector<QPair<double, double>> rangeQuery(double lo, double hi) const;

    /** @brief Force flush all buffered insertions */
    void flushBuffers();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void bulkLoadCompleted(int numKeys, int height, double timeMs);
    void bufferFlushed(int count, int level, double timeMs);

private:
    int m_minDegree = 4;      // t: node has [t-1, 2t-1] keys
    int m_bufferCapacity = 256;

    /** @brief B-tree node stored in pool */
    struct Node {
        QVector<double> keys;
        QVector<double> values;
        QVector<int> children;
        int parent = -1;
        bool isLeaf = true;
        // Buffer for lazy flushing
        QVector<QPair<double, double>> buffer;
    };

    QVector<Node> m_nodes;
    int m_root = -1;
    int m_freeList = -1;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Allocate a new node */
    int allocNode();

    /** @brief Free a node */
    void freeNode(int idx);

    /** @brief Split full child during insertion */
    void splitChild(int parentIdx, int childPos);

    /** @brief Insert into non-full node */
    void insertNonFull(int nodeIdx, double key, double value);

    /** @brief Flush buffer from node downward */
    void flushNodeBuffer(int nodeIdx);

    /** @brief Search in subtree rooted at nodeIdx */
    double searchIn(int nodeIdx, double key) const;

    /** @brief Range query in subtree */
    void rangeIn(int nodeIdx, double lo, double hi,
                 QVector<QPair<double, double>>& result) const;

    /** @brief Remove from subtree */
    bool removeFrom(int nodeIdx, double key);

    /** @brief Get predecessor key-value */
    QPair<double, double> predecessor(int nodeIdx, int keyIdx) const;

    /** @brief Get successor key-value */
    QPair<double, double> successor(int nodeIdx, int keyIdx) const;

    /** @brief Fill child that has fewer than t-1 keys */
    void fillChild(int nodeIdx, int childPos);

    /** @brief Borrow from left sibling */
    void borrowFromLeft(int nodeIdx, int childPos);

    /** @brief Borrow from right sibling */
    void borrowFromRight(int nodeIdx, int childPos);

    /** @brief Merge two children */
    void mergeChildren(int nodeIdx, int childPos);

    /** @brief Build tree from sorted leaf runs bottom-up */
    int buildFromSorted(const QVector<double>& keys,
                         const QVector<double>& values,
                         int start, int end);
};
