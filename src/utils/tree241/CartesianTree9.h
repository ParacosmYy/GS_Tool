/**
 * @file CartesianTree9.h
 * @brief 笛卡尔树(Treap式随机堆优先级+中序遍历范围最小查询) — Cartesian Tree with Treap-Style Randomized Heap Priority and In-Order Traversal for Range Minimum Queries
 *
 * 功能: 实现笛卡尔树(Cartesian tree)，采用Treap式随机堆优先级(Treap-style randomized heap
 *       priority)为每个元素分配随机优先级构建堆有序二叉树，通过中序遍历(in-order traversal)
 *       保持原始序列顺序，支持O(log n)范围最小查询(range minimum query)。
 *
 * 协作: FenwickTree8(树状数组) / SegmentTree7(线段树) / SplayTree10(伸展树)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 笛卡尔树(Treap式随机堆优先级+中序遍历范围最小查询)
 */
class CartesianTree9 : public QObject {
    Q_OBJECT

public:
    /** @brief Node in the Cartesian tree */
    struct Node {
        double value = 0.0;
        int priority = 0;
        int index = -1;
        int left = -1;
        int right = -1;
        int parent = -1;
        int subtreeMin = -1;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int treeSize = 0;
        int numQueries = 0;
        int numBuilds = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit CartesianTree9(QObject *parent = nullptr);
    ~CartesianTree9() override;

    /** @brief Build tree from value array */
    void build(const QVector<double>& values);

    /** @brief Range minimum query on [l, r] inclusive, returns value */
    double rangeMin(int l, int r) const;

    /** @brief Range minimum query on [l, r], returns original index */
    int rangeMinIndex(int l, int r) const;

    /** @brief Get node by internal index */
    Node node(int idx) const;

    /** @brief Get root index */
    int root() const;

    /** @brief In-order traversal returning indices */
    QVector<int> inOrder() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void buildCompleted(int size, double timeMs);
    void queryCompleted(int l, int r, double minVal, double timeMs);

private:
    QVector<Node> m_nodes;
    int m_root = -1;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Push subtree min into node */
    void pushUp(int idx);

    /** @brief Recursive in-order helper */
    void inOrderHelper(int idx, QVector<int>& result) const;

    /** @brief LCA-based range minimum index lookup */
    int lcaMin(int u, int v) const;
};
