/**
 * @file CartesianTree13.h
 * @brief 笛卡尔树(堆序栈构造与RMQ归约实现线性时间范围最小查询预处理) — Cartesian Tree with Heap-ordered Stack Construction and RMQ Reduction for Linear-time Range Minimum Query Preprocessing
 *
 * 功能: 实现笛卡尔树(Cartesian tree)，采用堆序栈构造(heap-ordered stack construction)
 *       与RMQ归约(RMQ reduction)实现线性时间范围最小查询预处理(linear-time range minimum query preprocessing)。
 *
 * 协作: SegmentTree13(线段树) / SplayTree14(伸展树) / FenwickTree12(树状数组)
 */
#pragma once

#include <QObject>
#include <QVector>

class CartesianTree13 : public QObject {
    Q_OBJECT

public:
    /** @brief Tree node */
    struct Node {
        double value = 0.0;
        int index = -1;
        int left = -1;
        int right = -1;
        int parent = -1;
    };

    /** @brief Build and query result */
    struct QueryResult {
        double minValue = 0.0;
        int minIndex = -1;
        double queryTimeMs = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalBuilds = 0;
        quint64 totalQueries = 0;
        int treeSize = 0;
        double avgBuildTimeMs = 0.0;
        double avgQueryTimeMs = 0.0;
    };

    explicit CartesianTree13(QObject *parent = nullptr);
    ~CartesianTree13() override;

    /** @brief Build Cartesian tree from value array (linear-time stack construction) */
    void build(const QVector<double>& values);

    /** @brief Range minimum query on [l, r] using Euler tour + sparse table */
    QueryResult rangeMinQuery(int l, int r) const;

    /** @brief Get tree nodes */
    const QVector<Node>& nodes() const { return m_nodes; }

    /** @brief Get tree root index */
    int root() const { return m_root; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void buildDone(int n, double timeMs);
    void queryDone(int l, int r, double minVal, double timeMs);

private:
    QVector<Node> m_nodes;
    int m_root = -1;
    Stats m_stats;
    double m_buildTimeSum = 0.0;
    double m_queryTimeSum = 0.0;

    /** @brief Euler tour sequence */
    QVector<int> m_euler;
    QVector<int> m_eulerDepth;
    QVector<int> m_firstOcc;

    /** @brief Sparse table for RMQ on Euler tour */
    QVector<QVector<int>> m_sparseTable;

    /** @brief Build Euler tour via DFS */
    void buildEulerTour();

    /** @brief Build sparse table for RMQ */
    void buildSparseTable();

    /** @brief DFS helper for Euler tour */
    void dfs(int node, int depth);
};
