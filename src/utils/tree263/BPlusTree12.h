/**
 * @file BPlusTree12.h
 * @brief B+树(排序输入批量加载链式叶子高效范围查询扫描) — B+ Tree with Bulk-loading from Sorted Input and Linked-leaf Chain for Efficient Range Query Scans
 *
 * 功能: 实现B+树(B+ tree)数据结构，支持从排序输入批量加载(bulk-loading from
 *       sorted input)和链式叶子节点(linked-leaf chain)进行高效范围查询扫描
 *       (efficient range query scans)。
 *
 * 协作: AvlTree9(AVL树) / RedBlackTree9(红黑树) / BTree10(B树)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief B+树(排序输入批量加载链式叶子高效范围查询扫描)
 */
class BPlusTree12 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numKeys = 0;
        int numNodes = 0;
        int treeHeight = 0;
        int order = 4;
        double avgProcessingTimeMs = 0.0;
    };

    explicit BPlusTree12(int order = 4, QObject *parent = nullptr);
    ~BPlusTree12() override;

    /** @brief Insert a key-value pair */
    void insert(int key, double value);

    /** @brief Remove a key */
    void remove(int key);

    /** @brief Search for exact key, returns value or NaN */
    double search(int key) const;

    /** @brief Range query: all key-value pairs in [lo, hi] via leaf chain scan */
    QVector<QPair<int, double>> rangeQuery(int lo, int hi) const;

    /** @brief Bulk-load from sorted key-value pairs */
    void bulkLoad(const QVector<QPair<int, double>>& sortedData);

    /** @brief Check if key exists */
    bool contains(int key) const;

    /** @brief Get all keys in sorted order */
    QVector<int> keys() const;

    /** @brief Get tree height */
    int height() const;

    /** @brief Get number of keys */
    int size() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void treeUpdated(int numKeys, int height, int numNodes, double timeMs);

private:
    /** @brief B+ tree node */
    struct Node {
        bool isLeaf = true;
        QVector<int> keys;
        QVector<double> values;     // leaf only
        QVector<Node*> children;    // internal only
        Node* next = nullptr;       // leaf chain pointer
        Node* parent = nullptr;
    };

    int m_order;        // max children per internal node
    Node* m_root = nullptr;
    Node* m_leftmostLeaf = nullptr;
    int m_size = 0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Find leaf node for key */
    Node* findLeaf(int key) const;

    /** @brief Split leaf node */
    Node* splitLeaf(Node* leaf);

    /** @brief Split internal node */
    Node* splitInternal(Node* node);

    /** @brief Insert into parent after child split */
    void insertIntoParent(Node* left, int key, Node* right);

    /** @brief Remove from leaf and handle underflow */
    void removeFromLeaf(Node* leaf, int idx);

    /** @brief Merge or redistribute nodes on underflow */
    void handleUnderflow(Node* node);

    /** @brief Find leftmost leaf */
    Node* findLeftmost() const;

    /** @brief Compute tree height */
    int computeHeight(Node* root) const;

    /** @brief Delete all nodes recursively */
    void clearTree(Node* node);

    /** @brief Count total nodes */
    int countNodes(Node* node) const;
};
