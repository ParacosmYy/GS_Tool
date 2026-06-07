/**
 * @file BPlusTree8.h
 * @brief B+树(排序输入批量加载+分数级联范围查询) — B+ Tree with Bulk Loading from Sorted Input and Fractional Cascading for Range Queries
 *
 * 功能: 实现B+树索引结构，支持排序输入批量加载、
 *       分数级联加速的范围查询和顺序遍历。
 *
 * 协作: AvlTree5(AVL树) / RedBlackTree8(红黑树) / BPlusTree7(B+树基础)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief B+树(排序输入批量加载+分数级联范围查询)
 */
class BPlusTree8 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int treeSize = 0;
        int treeHeight = 0;
        int numNodes = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit BPlusTree8(QObject *parent = nullptr);
    ~BPlusTree8() override;

    void setOrder(int order);

    /** @brief Bulk load from sorted key-value pairs */
    void bulkLoad(const QVector<QPair<int, double>>& sortedData);

    /** @brief Insert a key-value pair */
    void insert(int key, double value);

    /** @brief Remove a key */
    void remove(int key);

    /** @brief Look up value by exact key */
    QPair<bool, double> lookup(int key) const;

    /** @brief Range query with fractional cascading acceleration */
    QVector<QPair<int, double>> rangeQuery(int lo, int hi) const;

    /** @brief Get all key-value pairs in sorted order */
    QVector<QPair<int, double>> inOrderTraversal() const;

    /** @brief Check if tree is empty */
    bool isEmpty() const;

    /** @brief Clear all data */
    void clear();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void operationCompleted(const QString& op, int key, double timeMs);

private:
    int m_order = 64;  // Max children per internal node

    /** @brief B+ tree node stored in pool */
    struct Node {
        bool isLeaf = true;
        QVector<int> keys;
        QVector<double> values;      // Only for leaf nodes
        QVector<int> children;       // Indices into node pool (internal only)
        int next = -1;               // Leaf linked list
        int parent = -1;
        // Fractional cascading: for internal nodes, store bridge pointers
        QVector<int> bridgeIndices;  // Precomputed cascade hints
    };

    QVector<Node> m_nodes;
    int m_root = -1;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Allocate a new node, return index */
    int allocNode(bool leaf);

    /** @brief Find leaf node containing key */
    int findLeaf(int key) const;

    /** @brief Split a leaf node */
    int splitLeaf(int idx);

    /** @brief Split an internal node */
    int splitInternal(int idx);

    /** @brief Insert into parent after child split */
    void insertIntoParent(int left, int right, int key);

    /** @brief Build fractional cascading bridges */
    void buildCascadingBridges(int nodeIdx);

    /** @brief Range search within a leaf */
    void leafRangeCollect(int leaf, int lo, int hi,
                           QVector<QPair<int, double>>& result) const;
};
