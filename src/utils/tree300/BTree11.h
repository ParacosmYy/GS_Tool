/**
 * @file BTree11.h
 * @brief B树(写优化缓冲树与延迟刷新实现摊还高效批量插入及范围查询支持) — B-Tree with Write-Optimized Buffer Tree and Lazy Flushing for Amortized Efficient Bulk Insert with Range Query Support
 *
 * 功能: 实现B树(B-tree)，采用写优化缓冲树(write-optimized buffer tree)
 *       与延迟刷新(lazy flushing)实现摊还高效批量插入及范围查询支持(amortized efficient bulk insert with range query support)。
 *
 * 协作: Treap17(树堆) / RedBlackTree(红黑树) / LSMTree(LSM树)
 */
#pragma once

#include <QObject>
#include <QVector>

class BTree11 : public QObject {
    Q_OBJECT

public:
    /** @brief B-tree node with buffer for write optimization */
    struct Node {
        QVector<double> keys;
        QVector<int> children;    // child node indices (-1 = leaf)
        QVector<double> buffer;   // pending insertions (lazy flush)
        int parent = -1;
        bool isLeaf = true;
        int bufferSize = 0;
    };

    /** @brief Range query result */
    struct RangeResult {
        QVector<double> keys;
        int count = 0;
    };

    /** @brief Search result */
    struct SearchResult {
        bool found = false;
        double key = 0.0;
        int nodeIndex = -1;
        int keyIndex = -1;
        int depth = 0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalInserts = 0;
        quint64 totalFlushes = 0;
        quint64 totalSearches = 0;
        int treeSize = 0;
        int treeHeight = 0;
        int bufferSize = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit BTree11(QObject *parent = nullptr);
    ~BTree11() override;

    void setOrder(int t);  // minimum degree
    void setBufferSize(int size);

    /** @brief Insert a key with buffer-tree lazy flushing */
    bool insert(double key);

    /** @brief Search for a key (triggers buffer flush if needed) */
    SearchResult search(double key);

    /** @brief Range query [lo, hi] */
    RangeResult rangeQuery(double lo, double hi) const;

    /** @brief Flush all pending buffers */
    void flushAll();

    /** @brief In-order traversal of all keys */
    QVector<double> inOrder() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void insertDone(double key, int size, double timeMs);
    void flushDone(int flushed, double timeMs);

private:
    int m_t = 8;            // minimum degree (order)
    int m_maxBufferSize = 64;
    QVector<Node> m_nodes;
    int m_root = -1;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Allocate a new node */
    int allocNode(bool isLeaf);

    /** @brief Split a full child node */
    void splitChild(int parentIdx, int childPos);

    /** @brief Insert into a non-full node */
    void insertNonFull(int nodeIdx, double key);

    /** @brief Flush buffer of a node down the tree */
    void flushBuffer(int nodeIdx);

    /** @brief Check if buffer needs flushing */
    bool needsFlush(int nodeIdx) const;

    /** @brief Search helper (recursive) */
    SearchResult searchHelper(int nodeIdx, double key, int depth) const;

    /** @brief In-order traversal helper */
    void inOrderHelper(int idx, QVector<double>& result) const;

    /** @brief Range query helper */
    void rangeHelper(int idx, double lo, double hi, QVector<double>& result) const;
};
