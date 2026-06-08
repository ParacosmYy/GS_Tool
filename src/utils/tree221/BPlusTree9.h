/**
 * @file BPlusTree9.h
 * @brief B+树(分数级联叶链+前缀压缩内部键字符串索引) — B+ Tree with Fractional Cascading Leaf Links and Prefix-Compressed Internal Keys for String Index
 *
 * 功能: 实现B+树索引结构，叶子节点通过分数级联加速范围查询，
 *       内部节点使用前缀压缩减少字符串键存储开销。
 *
 * 协作: AvlTree6(AVL树) / RedBlackTree5(红黑树) / Trie8(字典树)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>

/**
 * @brief B+树(分数级联叶链+前缀压缩键)
 */
class BPlusTree9 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int nodeCount = 0;
        int leafCount = 0;
        int treeHeight = 0;
        int numInserts = 0;
        int numDeletes = 0;
        int totalKeys = 0;
        double prefixSavings = 0.0;  // Bytes saved by prefix compression
        double avgProcessingTimeMs = 0.0;
    };

    explicit BPlusTree9(QObject *parent = nullptr);
    ~BPlusTree9() override;

    /** @brief Set B+ tree order (max children per internal node) */
    void setOrder(int order = 64);

    /** @brief Insert a string key with integer value */
    void insert(const QString& key, int value);

    /** @brief Remove a key, returns true if found */
    bool remove(const QString& key);

    /** @brief Search for exact key, returns value or -1 */
    int search(const QString& key) const;

    /** @brief Range query: find all keys in [lo, hi) with fractional cascading */
    QVector<QPair<QString, int>> rangeQuery(const QString& lo, const QString& hi) const;

    /** @brief Get all keys in sorted order */
    QVector<QString> allKeys() const;

    /** @brief Clear all entries */
    void clear();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void operationCompleted(const QString& op, int key, double timeMs);

private:
    /** @brief Prefix-compressed key stored in internal nodes */
    struct CompressedKey {
        QString prefix;    // Common prefix
        QString suffix;    // Differentiating suffix
        int prefixLen = 0; // Length of shared prefix with separator
    };

    struct Node {
        bool isLeaf = false;
        int numKeys = 0;
        Node* parent = nullptr;

        // Leaf data
        QVector<QString> keys;
        QVector<int> values;
        Node* next = nullptr;         // Leaf link for sequential scan
        QVector<int> fracCascIdx;     // Fractional cascading indices

        // Internal data
        QVector<CompressedKey> compKeys;
        QVector<Node*> children;

        ~Node();
    };

    int m_order = 64;
    Node* m_root = nullptr;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute compressed key between two separator strings */
    CompressedKey compressKey(const QString& lo, const QString& hi) const;

    /** @brief Find leaf node that should contain key */
    Node* findLeaf(const QString& key) const;

    /** @brief Split an overflowing leaf node */
    Node* splitLeaf(Node* leaf);

    /** @brief Split an overflowing internal node */
    Node* splitInternal(Node* node);

    /** @brief Insert separator into parent after child split */
    void insertIntoParent(Node* left, const QString& key, Node* right);

    /** @brief Update compressed keys along a path */
    void updateCompression(Node* node);

    /** @brief Build fractional cascading index for leaf */
    void buildFracCascIdx(Node* leaf) const;

    /** @brief Find position in leaf using fractional cascading hint */
    int fracCascSearch(Node* leaf, const QString& key, int hint) const;

    /** @brief Compute tree height */
    int computeHeight() const;

    /** @brief Collect all leaf nodes for rebuild */
    void collectLeaves(Node* node, QVector<Node*>& leaves) const;

    /** @brief Recursive node cleanup */
    void clearNode(Node* node);
};
