/**
 * @file CartesianTree7.h
 * @brief 笛卡尔树(Treap构造+Euler游程+稀疏表范围最小查询) — Cartesian Tree with Treap-Based Construction and Range Minimum Query via Euler Tour with Sparse Table
 *
 * 功能: 实现笛卡尔树，支持Treap式构造、
 *       Euler游程转换和稀疏表RMQ查询。
 *
 * 协作: FenwickTree6(树状数组) / SegmentTree9(线段树) / SparseTable5(稀疏表)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 笛卡尔树(Treap构造+Euler游程+稀疏表范围最小查询)
 */
class CartesianTree7 : public QObject {
    Q_OBJECT

public:
    /** @brief Tree node */
    struct Node {
        double value = 0.0;    // Value (heap property)
        int priority = 0;      // Priority (treap: BST property)
        int index = -1;        // Original index
        int left = -1;         // Left child index
        int right = -1;        // Right child index
        int parent = -1;       // Parent index
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalQueries = 0;
        int treeSize = 0;
        int eulerTourLength = 0;
        int treeHeight = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit CartesianTree7(QObject *parent = nullptr);
    ~CartesianTree7() override;

    /** @brief Build Cartesian tree using treap-based construction (O(n)) */
    void build(const QVector<double>& values);

    /** @brief Build with explicit priorities */
    void buildWithPriorities(const QVector<QPair<double, int>>& entries);

    /** @brief Range minimum query [l..r] using Euler tour + sparse table */
    QPair<double, int> rangeMinimumQuery(int l, int r) const;

    /** @brief Get tree nodes */
    QVector<Node> nodes() const;

    /** @brief Get root index */
    int root() const;

    /** @brief Get Euler tour sequence */
    QVector<int> eulerTour() const;

    /** @brief Get depth of each position in Euler tour */
    QVector<int> eulerDepths() const;

    /** @brief Compute tree height */
    int computeHeight() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void treeBuilt(int size, int height, double timeMs);
    void queryCompleted(int l, int r, double minValue, double timeMs);

private:
    QVector<Node> m_nodes;
    int m_root = -1;

    // Euler tour + sparse table for RMQ
    QVector<int> m_euler;       // Euler tour node indices
    QVector<int> m_eulerDepth;  // Depth at each Euler position
    QVector<int> m_firstOcc;    // First occurrence of each node in Euler
    QVector<QVector<int>> m_sparseTable;  // Sparse table for depth RMQ
    QVector<int> m_logTable;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build Euler tour via DFS */
    void buildEulerTour();

    /** @brief Build sparse table for RMQ on Euler depths */
    void buildSparseTable();

    /** @brief Compute subtree height */
    int subtreeHeight(int nodeIdx) const;

    /** @brief Log2 lookup table builder */
    static QVector<int> buildLogTable(int n);
};
