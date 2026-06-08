/**
 * @file Rope8.h
 * @brief 绳索结构(平衡二叉叶缓冲树+Fibonacci权重增量再平衡分裂) — Rope with Balanced Binary Tree of Leaf Buffers and Incremental Rebalancing via Fibonacci-weight Splitting
 *
 * 功能: 实现绳索(Rope)数据结构，采用平衡二叉树管理叶缓冲区(leaf buffers)，
 *       通过Fibonacci权重(Fibonacci-weight)进行增量再平衡(incremental rebalancing)与分裂。
 *
 * 协作: Treap11(树堆) / AVLTree5(AVL树) / BPlusTree3(B+树)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 绳索结构(平衡二叉叶缓冲+Fibonacci权重再平衡)
 */
class Rope8 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numNodes = 0;
        int treeHeight = 0;
        int totalLength = 0;
        int numRebalances = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Rope8(QObject *parent = nullptr);
    ~Rope8() override;

    /** @brief Build rope from string */
    void build(const QString& text);

    /** @brief Insert string at position */
    void insert(int pos, const QString& text);

    /** @brief Delete range [from, to) */
    void remove(int from, int to);

    /** @brief Extract substring [from, to) */
    QString substring(int from, int to) const;

    /** @brief Character at index */
    QChar charAt(int index) const;

    /** @brief Total character count */
    int length() const;

    /** @brief Convert entire rope to string */
    QString toString() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void ropeUpdated(int length, int height, double timeMs);

private:
    /** @brief Rope node stored in pool */
    struct Node {
        int weight = 0;       // Fibonacci-weight for balancing
        int leftLen = 0;      // length of left subtree
        int left = -1;
        int right = -1;
        int parent = -1;
        bool isLeaf = false;
        QString leafData;     // valid only if isLeaf
    };

    QVector<Node> m_nodes;
    int m_root = -1;
    int m_freeList = -1;
    int m_leafCapacity = 64;  // max chars per leaf

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Allocate node */
    int allocNode();

    /** @brief Free node */
    void freeNode(int idx);

    /** @brief Update leftLen up the tree */
    void updatePath(int idx);

    /** @brief Compute Fibonacci weight from node position */
    int fibWeight(int idx) const;

    /** @brief Check and rebalance if needed */
    int rebalance(int root);

    /** @brief Split node at internal offset */
    int splitLeaf(int idx, int offset);

    /** @brief Concat two subtrees into new root */
    int concat(int left, int right);

    /** @brief Collect leaves in order */
    void collectLeaves(int idx, QVector<int>& leaves) const;

    /** @brief Find leaf and offset for character index */
    int findLeaf(int root, int& offset) const;

    /** @brief Compute tree height */
    int heightOf(int idx) const;

    /** @brief Compute total length of subtree */
    int lengthOf(int idx) const;
};

