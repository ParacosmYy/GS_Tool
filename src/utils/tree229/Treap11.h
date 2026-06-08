/**
 * @file Treap11.h
 * @brief 树堆(合并分裂优先队列+增强子树大小K阶统计量选择) — Treap with Merge-split Priority Queue and K-th Order Statistic Selection via Augmented Subtree Size
 *
 * 功能: 实现树堆(Treap)数据结构，支持合并(merge)和分裂(split)操作的优先队列，
 *       通过增强子树大小(augmented subtree size)支持K阶统计量(K-th order statistic)选择。
 *
 * 协作: AVLTree5(AVL树) / RedBlackTree6(红黑树) / SplayTree4(伸展树)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 树堆(合并分裂优先队列+K阶统计量选择)
 */
class Treap11 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numNodes = 0;
        int treeHeight = 0;
        int numMerges = 0;
        int numSplits = 0;
        int numKthQueries = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Treap11(QObject *parent = nullptr);
    ~Treap11() override;

    /** @brief Insert key-value pair */
    void insert(int key, double value = 0.0);

    /** @brief Remove key */
    bool remove(int key);

    /** @brief Search for key, return value (NaN if not found) */
    double search(int key) const;

    /** @brief Select K-th smallest element (1-indexed) */
    int selectKth(int k) const;

    /** @brief Get rank of key (number of elements < key) */
    int rank(int key) const;

    /** @brief Split tree by key into [<=key] and [>key] */
    void split(int key, Treap11& left, Treap11& right);

    /** @brief Merge two treaps (all keys in left < all keys in right) */
    void merge(Treap11& left, Treap11& right);

    /** @brief Get all keys in sorted order */
    QVector<int> inorderKeys() const;

    int size() const { return m_stats.numNodes; }
    int height() const;
    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void treeUpdated(int nodes, int height, double timeMs);

private:
    /** @brief Treap node */
    struct Node {
        int key = 0;
        double value = 0.0;
        int priority = 0;
        int left = -1;
        int right = -1;
        int subtreeSize = 1;
    };

    QVector<Node> m_nodes;
    int m_root = -1;
    int m_freeList = -1;
    int m_seed = 42;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Allocate node */
    int allocateNode(int key, double value);

    /** @brief Free node */
    void freeNode(int idx);

    /** @brief Update subtree size */
    void updateSize(int idx);

    /** @brief Get subtree size */
    int getSize(int idx) const;

    /** @brief Internal merge returning root index */
    int mergeInternal(int left, int right);

    /** @brief Internal split */
    void splitInternal(int root, int key, int& left, int& right);

    /** @brief Internal insert */
    int insertInternal(int root, int nodeIdx);

    /** @brief Internal remove */
    int removeInternal(int root, int key);

    /** @brief Internal K-th select */
    int selectKthInternal(int root, int k) const;

    /** @brief Internal rank query */
    int rankInternal(int root, int key) const;

    /** @brief Inorder traversal */
    void inorderHelper(int idx, QVector<int>& keys) const;

    /** @brief Compute height */
    int heightHelper(int idx) const;

    /** @brief Random priority */
    int randomPriority();
};
