/**
 * @file CartesianTree11.h
 * @brief 笛卡尔树(Treap堆性质与中序序列保持范围最小查询预处理) — Cartesian Tree with Treap-Style Heap Property and In-Order Sequence Preservation for Range Minimum Query Preprocessing
 *
 * 功能: 实现笛卡尔树(Cartesian tree)，采用Treap式堆性质(treap-style heap property)
 *       与中序序列保持(in-order sequence preservation)用于范围最小查询预处理(RMQ preprocessing)。
 *
 * 协作: SegmentTree13(线段树) / SplayTree12(伸展树) / FenwickTree10(树状数组)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 笛卡尔树(Treap堆性质与中序序列保持范围最小查询预处理)
 */
class CartesianTree11 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int treeSize = 0;
        int treeHeight = 0;
        int numQueries = 0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Tree node with treap-style index and value */
    struct Node {
        int index = -1;     // in-order position
        double value = 0.0; // heap key
        int parent = -1;
        int left = -1;
        int right = -1;
    };

    explicit CartesianTree11(QObject *parent = nullptr);
    ~CartesianTree11() override;

    /** @brief Build Cartesian tree from value sequence (linear-time stack method) */
    void build(const QVector<double>& values);

    /** @brief Query range minimum value in [l, r] */
    double rangeMinimum(int l, int r) const;

    /** @brief Query index of range minimum in [l, r] */
    int rangeMinimumIndex(int l, int r) const;

    /** @brief Get node by index */
    Node nodeAt(int index) const;

    /** @brief Get root index */
    int root() const;

    /** @brief Get Euler tour for LCA preprocessing */
    QVector<int> eulerTour() const;

    /** @brief Compute tree height */
    int height() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void treeBuilt(int size, int height, double timeMs);

private:
    int m_root = -1;
    QVector<Node> m_nodes;

    // Sparse table for RMQ on Euler tour (for LCA-based RMQ)
    QVector<QVector<int>> m_sparseTable;
    QVector<int> m_eulerTour;
    QVector<int> m_eulerDepth;
    QVector<int> m_firstOcc;    // first occurrence of node in Euler tour

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build Euler tour via DFS */
    void buildEulerTour();

    /** @brief Build sparse table for RMQ on depths */
    void buildSparseTable();

    /** @brief Compute node depth */
    int depth(int nodeIndex) const;

    /** @brief LCA via sparse table on Euler tour */
    int lca(int u, int v) const;

    /** @brief RMQ on sparse table (minimum depth in range) */
    int sparseRMQ(int l, int r) const;
};
