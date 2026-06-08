/**
 * @file CartesianTree8.h
 * @brief 笛卡尔树(分治构建+二进制提升LCA查询O(1)稀疏表) — Cartesian Tree with Divide-and-Conquer Construction and LCA Query via Binary Lifting with O(1) Sparse Table
 *
 * 功能: 实现笛卡尔树(Cartesian Tree)的分治法构建，支持LCA(最近公共祖先)查询，
 *       通过二进制提升(binary lifting)预处理和O(1)稀疏表实现高效LCA回答。
 *
 * 协作: SegmentTree6(线段树) / SplayTree9(伸展树) / FenwickTree7(树状数组)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 笛卡尔树(分治构建+二进制提升LCA)
 */
class CartesianTree8 : public QObject {
    Q_OBJECT

public:
    /** @brief Tree node */
    struct Node {
        int value = 0;       // node value (heap key)
        int index = 0;       // original array index (inorder key)
        int parent = -1;
        int left = -1;
        int right = -1;
    };

    /** @brief LCA query result */
    struct LCAResult {
        int lcaNode = -1;
        int lcaIndex = -1;
        int lcaValue = 0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int treeSize = 0;
        int treeHeight = 0;
        int numLCAQueries = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit CartesianTree8(QObject *parent = nullptr);
    ~CartesianTree8() override;

    /** @brief Build Cartesian tree from array (min-heap property) */
    bool build(const QVector<int>& values);

    /** @brief Query LCA of two nodes (by original indices) */
    LCAResult lca(int indexA, int indexB) const;

    /** @brief Query LCA of range [l, r] (RMQ via Cartesian tree) */
    LCAResult rangeMinimum(int l, int r) const;

    /** @brief Get tree height */
    int height() const;

    /** @brief Get inorder traversal */
    QVector<int> inorder() const;

    /** @brief Get all nodes */
    const QVector<Node>& nodes() const { return m_nodes; }

    int size() const { return m_treeSize; }
    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void treeBuilt(int size, int height, double timeMs);
    void lcaQueried(int u, int v, int lca, double timeMs);

private:
    int m_treeSize = 0;
    int m_root = -1;
    int m_maxLog = 0;

    QVector<Node> m_nodes;
    // Map original index -> node id
    QVector<int> m_indexToNode;

    // Binary lifting table: up[node][k] = 2^k-th ancestor
    QVector<QVector<int>> m_up;

    // Depth of each node
    QVector<int> m_depth;

    // Sparse table for O(1) LCA via Euler tour
    QVector<int> m_euler;
    QVector<int> m_eulerDepth;
    QVector<int> m_firstOccurrence;
    QVector<QVector<int>> m_sparseTable;
    QVector<int> m_logTable;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Divide-and-conquer Cartesian tree builder */
    int buildDC(const QVector<int>& values, int lo, int hi);

    /** @brief Build binary lifting table */
    void buildLifting();

    /** @brief Build Euler tour and sparse table */
    void buildEulerSparse();

    /** @brief DFS for Euler tour */
    void eulerDFS(int node, int depth);

    /** @brief LCA via binary lifting */
    int lcaLifting(int u, int v) const;

    /** @brief RMQ on Euler depth array using sparse table */
    int rmqSparse(int l, int r) const;
};
