/**
 * @file CartesianTree6.h
 * @brief 笛卡尔树(堆序优先级+Euler游历+稀疏表RMQ) — Cartesian Tree with Heap-Ordered Priority, Range Minimum Query via Euler Tour + Sparse Table
 *
 * 功能: 实现笛卡尔树，支持堆序优先级构建、Euler游历序列、
 *       稀疏表O(1)区间最小查询和LCA查询。
 *
 * 协作: SegmentTree8(线段树) / SuffixArray5(后缀数组) / AVLTree4(AVL树)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 笛卡尔树(堆序优先级+Euler游历+稀疏表RMQ)
 */
class CartesianTree6 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalOperations = 0;
        int numNodes = 0;
        int treeHeight = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit CartesianTree6(QObject *parent = nullptr);
    ~CartesianTree6() override;

    /** @brief 从值数组构建笛卡尔树(值=优先级, 索引=中序键) */
    void build(const QVector<double>& values);

    /** @brief 查询区间[l, r]的最小值位置(RMQ) */
    int rangeMinimumQuery(int l, int r) const;

    /** @brief 查询区间[l, r]的最小值 */
    double rangeMinimum(int l, int r) const;

    /** @brief 查询两点的LCA */
    int lca(int u, int v) const;

    /** @brief 获取节点值的数组 */
    QVector<double> values() const { return m_values; }

    /** @brief 获取Euler游历序列 */
    QVector<int> eulerTour() const { return m_euler; }

    /** @brief 获取Euler游历深度序列 */
    QVector<int> eulerDepth() const { return m_eulerDepth; }

    int size() const { return m_values.size(); }
    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void buildCompleted(int numNodes, int height, double timeMs);

private:
    /** @brief Tree node */
    struct Node {
        int index = -1;         // In-order index (array position)
        double priority = 0.0;  // Heap priority (value)
        int parent = -1;
        int left = -1;
        int right = -1;
        int depth = 0;
    };

    QVector<Node> m_nodes;
    QVector<double> m_values;
    int m_root = -1;

    Stats m_stats;
    double m_timeSum = 0.0;

    // Euler tour data
    QVector<int> m_euler;           // Node indices in Euler order
    QVector<int> m_eulerDepth;      // Depth at each Euler position
    QVector<int> m_firstOccurrence; // First Euler position for each node

    // Sparse table for RMQ on Euler depths
    QVector<QVector<int>> m_sparseTable;
    QVector<int> m_logTable;

    /** @brief Build tree using stack-based O(n) algorithm */
    void buildTree(const QVector<double>& values);

    /** @brief Perform Euler tour (DFS) */
    void performEulerTour();

    /** @brief DFS helper for Euler tour */
    void dfs(int node, int depth, int& eulerIdx);

    /** @brief Build sparse table for RMQ on Euler depths */
    void buildSparseTable();

    /** @brief Sparse table RMQ query (returns index of minimum depth) */
    int sparseQuery(int l, int r) const;

    /** @brief Compute tree height */
    int computeHeight(int node) const;
};
