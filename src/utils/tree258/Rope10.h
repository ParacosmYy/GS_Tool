/**
 * @file Rope10.h
 * @brief 绳索结构(平衡BST叶索引+字符级分裂合并增量再平衡) — Rope with Balanced BST Leaf Indexing and Character-level Split/Concat with Incremental Rebalance
 *
 * 功能: 实现绳索数据结构(Rope)，采用平衡BST叶索引(balanced BST leaf
 *       indexing)，支持字符级分裂/合并(character-level split/concat)
 *       配合增量再平衡(incremental rebalance)实现高效文本编辑操作。
 *
 * 协作: Treap13(树堆) / AVLTree8(AVL树) / BPlusTree7(B+树)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>

/**
 * @brief 绳索结构(平衡BST叶索引+字符级分裂合并增量再平衡)
 */
class Rope10 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numNodes = 0;
        int numLeaves = 0;
        int totalLength = 0;
        int numSplits = 0;
        int numConcats = 0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Rope node stored in pool */
    struct Node {
        int weight = 0;        // Character count in left subtree (or leaf length)
        int left = -1;         // Left child index
        int right = -1;        // Right child index
        int height = 1;        // AVL height for rebalance
        bool isLeaf = false;
        QString data;          // Leaf content
    };

    explicit Rope10(QObject *parent = nullptr);
    ~Rope10() override;

    /** @brief Build rope from string */
    void build(const QString& text);

    /** @brief Set maximum leaf size */
    void setLeafSize(int size);

    /** @brief Set rebalance threshold */
    void setRebalanceThreshold(int threshold);

    /** @brief Insert string at position */
    void insert(int pos, const QString& text);

    /** @brief Remove characters in range [pos, pos+len) */
    void remove(int pos, int len);

    /** @brief Get character at position */
    QChar charAt(int pos) const;

    /** @brief Extract substring [start, start+len) */
    QString substring(int start, int len) const;

    /** @brief Split rope at position, returns right part */
    Rope10* split(int pos);

    /** @brief Concatenate another rope into this one */
    void concat(Rope10* other);

    /** @brief Get full string content */
    QString toString() const;

    /** @brief Get total character count */
    int length() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void ropeUpdated(int numNodes, int totalLength, double timeMs);

private:
    int m_root = -1;
    int m_leafSize = 64;
    int m_rebalanceThreshold = 8;

    QVector<Node> m_nodes;
    int m_freeList = -1;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Allocate node */
    int allocNode();

    /** @brief Free node */
    void freeNode(int idx);

    /** @brief Create leaf node */
    int createLeaf(const QString& data);

    /** @brief Update node weight and height */
    void updateNode(int idx);

    /** @brief Get AVL balance factor */
    int balanceFactor(int idx) const;

    /** @brief Rotate left */
    int rotateLeft(int idx);

    /** @brief Rotate right */
    int rotateRight(int idx);

    /** @brief Rebalance subtree */
    int rebalance(int idx);

    /** @brief Split subtree at position */
    void splitHelper(int root, int pos, int& left, int& right);

    /** @brief Merge two subtrees */
    int mergeTrees(int left, int right);

    /** @brief Report subtree at index */
    void reportSubtree(int idx, QString& result) const;

    /** @brief Index into subtree */
    QChar indexInto(int idx, int pos) const;
};
