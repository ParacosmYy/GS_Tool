/**
 * @file CartesianTree10.h
 * @brief 笛卡尔树(原地线性时间右脊栈构建+range-min离线查询) — Cartesian Tree with In-Place Linear-Time Construction via Right-Spine Stack and Range-Minimum-Query Offline Processing
 *
 * 功能: 实现笛卡尔树(Cartesian Tree)，通过右脊栈(right-spine stack)以
 *       原地线性时间(in-place linear time O(n))构建，支持range-minimum
 *       query (RMQ)离线处理(offline processing)利用LCA最近公共祖先。
 *
 * 协作: FenwickTree9(树状数组) / SegmentTree8(线段树) / SplayTree11(伸展树)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 笛卡尔树(右脊栈线性构建+RMQ离线查询)
 */
class CartesianTree10 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numNodes = 0;
        int numQueries = 0;
        int numBuilds = 0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Tree node representation */
    struct Node {
        int value = 0;
        int index = -1;
        int parent = -1;
        int left = -1;
        int right = -1;
    };

    explicit CartesianTree10(QObject *parent = nullptr);
    ~CartesianTree10() override;

    /** @brief Build Cartesian tree from array (O(n) via right-spine stack) */
    void build(const QVector<int>& values);

    /** @brief Range-minimum query: return minimum value in [l, r] */
    int rangeMin(int l, int r) const;

    /** @brief Range-minimum query: return index of minimum in [l, r] */
    int rangeMinIndex(int l, int r) const;

    /** @brief Get the root index */
    int root() const;

    /** @brief Get all nodes */
    QVector<Node> nodes() const;

    /** @brief Get Euler tour for LCA-based RMQ */
    QVector<int> eulerTour() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void buildCompleted(int numNodes, double timeMs);
    void queryCompleted(int l, int r, int minValue, double timeMs);

private:
    int m_root = -1;
    int m_n = 0;
    QVector<Node> m_nodes;
    QVector<int> m_values;       // Original values for RMQ

    // Euler tour arrays for LCA-based RMQ
    QVector<int> m_euler;        // Euler tour node indices
    QVector<int> m_depth;        // Depth at each Euler tour position
    QVector<int> m_first;        // First occurrence of each node in Euler tour

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build Euler tour via DFS */
    void buildEulerTour();

    /** @brief DFS helper for Euler tour */
    void dfs(int node, int depth, int& pos);

    /** @brief LCA via sparse table on Euler tour (RMQ on depths) */
    int lca(int u, int v) const;

    /** @brief Build sparse table for RMQ on depth array */
    void buildSparseTable();
};
