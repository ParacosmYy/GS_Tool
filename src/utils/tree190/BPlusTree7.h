/**
 * @file BPlusTree7.h
 * @brief B+树(批量加载+叶子链范围扫描+缓冲插入写优化) — B+ Tree with Bulk Loading, Range Scan via Leaf Chain and Buffered Insert for Write-Optimized Workloads
 *
 * 功能: 实现B+树索引结构，支持批量加载(Bulk Loading)、
 *       叶子链表范围扫描、缓冲插入(Buffered Insert)写优化和磁盘友好节点大小。
 *
 * 协作: AvlTree4(AVL树) / RedBlackTree5(红黑树) / BPlusTree6(基本B+树)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief B+树(批量加载+叶子链范围扫描+缓冲插入)
 */
class BPlusTree7 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalOperations = 0;
        int numKeys = 0;
        int numNodes = 0;
        int treeHeight = 0;
        int bufferFlushes = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit BPlusTree7(int order = 32, QObject *parent = nullptr);
    ~BPlusTree7() override;

    /** @brief Insert a key-value pair */
    void insert(double key, double value);

    /** @brief Remove a key */
    void remove(double key);

    /** @brief Find value for key (qQNaN if not found) */
    double find(double key) const;

    /** @brief Range scan [lo, hi] using leaf chain */
    QVector<QPair<double, double>> rangeScan(double lo, double hi) const;

    /** @brief Bulk load from sorted key-value pairs */
    void bulkLoad(const QVector<QPair<double, double>>& sortedPairs);

    /** @brief Flush buffered inserts into the tree */
    void flushBuffer();

    /** @brief Buffered insert (deferred write) */
    void bufferedInsert(double key, double value);

    /** @brief Clear the tree */
    void clear();

    int size() const { return m_size; }
    int height() const { return m_height; }
    int order() const { return m_order; }
    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void operationCompleted(const QString& op, int numKeys, double timeMs);

private:
    /** @brief Tree node (internal or leaf) */
    struct Node {
        bool isLeaf = false;
        QVector<double> keys;
        QVector<Node*> children;   // Internal node children
        QVector<double> values;    // Leaf node values
        Node* next = nullptr;      // Leaf chain pointer
        Node* parent = nullptr;
    };

    int m_order;
    int m_size = 0;
    int m_height = 0;
    Node* m_root = nullptr;

    // Buffer for buffered inserts
    QVector<QPair<double, double>> m_buffer;
    int m_bufferThreshold = 256;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Find leaf node containing key */
    Node* findLeaf(double key) const;

    /** @brief Split an overfull leaf node */
    Node* splitLeaf(Node* leaf);

    /** @brief Split an overfull internal node */
    Node* splitInternal(Node* node);

    /** @brief Insert key into parent after child split */
    void insertIntoParent(Node* left, double key, Node* right);

    /** @brief Borrow from or merge with sibling */
    void rebalanceLeaf(Node* leaf);
    void rebalanceInternal(Node* node);

    /** @brief Recursive delete */
    void deleteTree(Node* n);

    /** @brief Find leftmost leaf */
    Node* leftmostLeaf() const;
};
