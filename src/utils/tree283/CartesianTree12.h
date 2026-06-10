/**
 * @file CartesianTree12.h
 * @brief 笛卡尔树(线性时间栈构造与Euler游程范围最小查询的LCA预处理) — Cartesian Tree with Linear-time Stack Construction and Range Minimum Query via Euler Tour for LCA Preprocessing
 *
 * 功能: 实现笛卡尔树(Cartesian tree)，采用线性时间栈构造(linear-time stack construction)
 *       与Euler游程范围最小查询(range minimum query via Euler tour)实现LCA预处理(LCA preprocessing)。
 *
 * 协作: SegmentTree10(线段树) / SparseTable9(稀疏表) / FenwickTree11(树状数组)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 笛卡尔树(线性时间栈构造与Euler游程范围最小查询)
 */
class CartesianTree12 : public QObject {
    Q_OBJECT

public:
    /** @brief Tree node */
    struct Node {
        int left = -1;
        int right = -1;
        int parent = -1;
        double value = 0.0;
        int index = -1;  // Original index in input array
    };

    /** @brief Query result for RMQ */
    struct RMQResult {
        double minValue = 0.0;
        int minIndex = -1;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int treeSize = 0;
        int numQueries = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit CartesianTree12(QObject *parent = nullptr);
    ~CartesianTree12() override;

    /** @brief Build Cartesian tree from array via linear-time stack construction */
    void build(const QVector<double>& values);

    /** @brief Range minimum query via Euler tour + sparse table */
    RMQResult rangeMinQuery(int l, int r) const;

    /** @brief Find lowest common ancestor of two nodes */
    int lca(int u, int v) const;

    /** @brief Get node by index */
    const Node& node(int idx) const;

    /** @brief Get tree root index */
    int root() const { return m_root; }

    /** @brief Get inorder traversal */
    QVector<int> inorderTraversal() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void buildDone(int n, int root, double timeMs);
    void queryDone(int l, int r, double minVal, int minIdx, double timeMs);

private:
    int m_root = -1;
    int m_n = 0;
    Stats m_stats;
    double m_timeSum = 0.0;

    QVector<Node> m_nodes;

    /** @brief Euler tour arrays */
    QVector<int> m_euler;      // Euler tour sequence (node indices)
    QVector<int> m_eulerDepth; // Depth at each Euler position
    QVector<int> m_firstOcc;   // First occurrence of each node in Euler tour

    /** @brief Sparse table for RMQ on Euler depths */
    QVector<QVector<int>> m_sparseTable;

    /** @brief Log2 lookup */
    QVector<int> m_log2;

    /** @brief Build Euler tour via DFS */
    void buildEulerTour();

    /** @brief Build sparse table for RMQ on Euler depths */
    void buildSparseTable();

    /** @brief DFS for Euler tour */
    void eulerDFS(int u, int depth, int& pos);

    /** @brief Sparse table RMQ query */
    int sparseQuery(int l, int r) const;

    /** @brief Inorder DFS */
    void inorderDFS(int u, QVector<int>& result) const;
};
