/**
 * @file BTree8.h
 * @brief B树(写优化缓冲树+分数级联批量插入范围查询) — B-tree with Write-optimized Buffer Tree and Fractional Cascading for Efficient Batch Insert and Range Queries
 *
 * 功能: 实现写优化B树(Buffer Tree)，采用缓冲树(buffer tree)延迟
 *       写入合并，配合分数级联(fractional cascading)优化范围查询
 *       (range queries)的效率，支持批量插入。
 *
 * 协作: BPlusTree7(B+树) / AVLTree8(AVL树) / Rope10(绳索结构)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief B树(写优化缓冲树+分数级联批量插入范围查询)
 */
class BTree8 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numNodes = 0;
        int numKeys = 0;
        int treeHeight = 0;
        int bufferSize = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit BTree8(int order = 64, QObject *parent = nullptr);
    ~BTree8() override;

    /** @brief Set B-tree order (max children per node) */
    void setOrder(int order);

    /** @brief Set buffer size for write optimization */
    void setBufferSize(int size);

    /** @brief Insert a single key-value pair */
    void insert(double key, double value);

    /** @brief Batch insert multiple key-value pairs */
    void batchInsert(const QVector<double>& keys, const QVector<double>& values);

    /** @brief Search for exact key, returns value (NaN if not found) */
    double search(double key) const;

    /** @brief Range query [lo, hi], returns keys and values */
    void rangeQuery(double lo, double hi, QVector<double>& outKeys,
                    QVector<double>& outValues) const;

    /** @brief Remove a key */
    bool remove(double key);

    /** @brief Flush all buffers to leaf nodes */
    void flushBuffers();

    /** @brief Get tree height */
    int height() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void insertCompleted(int numKeys, int height, double timeMs);
    void rangeCompleted(int resultSize, double timeMs);

private:
    /** @brief B-tree node with write buffer */
    struct Node {
        bool isLeaf = false;
        QVector<double> keys;
        QVector<double> values;
        QVector<int> children;    // Child node indices
        int parent = -1;
        // Write buffer for lazy propagation
        QVector<QPair<double, double>> buffer;
    };

    int m_order = 64;
    int m_bufferSize = 256;
    int m_root = -1;
    int m_height = 0;

    QVector<Node> m_nodes;
    int m_freeList = -1;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Allocate a new node */
    int allocNode(bool isLeaf);

    /** @brief Free a node */
    void freeNode(int idx);

    /** @brief Split an overflowing node */
    int splitNode(int idx);

    /** @brief Insert into node buffer, flush if full */
    void insertBuffer(int nodeIdx, double key, double value);

    /** @brief Flush buffer from node to children */
    void flushNodeBuffer(int nodeIdx);

    /** @brief Insert directly into sorted node keys */
    void insertDirect(int nodeIdx, double key, double value);

    /** @brief Find leaf node for key */
    int findLeaf(double key) const;

    /** @brief Range query helper with fractional cascading hint */
    void rangeHelper(int nodeIdx, double lo, double hi,
                     QVector<double>& outKeys, QVector<double>& outValues) const;

    /** @brief Binary search for key in node */
    int keyIndex(int nodeIdx, double key) const;
};
