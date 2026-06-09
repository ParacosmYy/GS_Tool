/**
 * @file VertexCover8.h
 * @brief 顶点覆盖(2-近似极大匹配+度数折叠分支归约) — Vertex Cover with 2-Approximation Maximal Matching and Branch-and-Reduce with Degree-Based Folding Rules
 *
 * 功能: 实现顶点覆盖(Vertex Cover)算法，使用2-近似极大匹配(2-approximation
 *       maximal matching)获得初始解，结合度数折叠规则(degree-based folding)
 *       的分支归约(branch-and-reduce)搜索精确解。
 *
 * 协作: MaxClique9(最大团) / GraphColoring7(图着色) / ShortestPath12(最短路径)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 顶点覆盖(2-近似极大匹配+度数折叠分支归约)
 */
class VertexCover8 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numVertices = 0;
        int numEdges = 0;
        int coverSize = 0;
        int approxCoverSize = 0;
        int numBranches = 0;
        int numFolds = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit VertexCover8(QObject *parent = nullptr);
    ~VertexCover8() override;

    /** @brief Build graph from edge list (pairs of vertex indices) */
    void setGraph(int numVertices, const QVector<QPair<int, int>>& edges);

    /** @brief Compute 2-approximation via maximal matching */
    QVector<int> approximate();

    /** @brief Compute exact vertex cover via branch-and-reduce */
    QVector<int> exact();

    /** @brief Get the size of the last computed cover */
    int coverSize() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void coverComputed(int coverSize, bool exact, double timeMs);

private:
    int m_n = 0;
    QVector<QVector<int>> m_adj;  // Adjacency list
    QVector<QPair<int, int>> m_edges;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Current best cover for branch-and-bound */
    QVector<int> m_bestCover;
    int m_bestSize = 0;

    /** @brief Greedy maximal matching */
    QVector<QPair<int, int>> maximalMatching() const;

    /** @brief Branch-and-reduce recursive search */
    void branchReduce(QVector<int>& cover, QVector<bool>& inCover,
                       QVector<bool>& removed, int depth);

    /** @brief Apply degree-0 and degree-1 folding rules */
    int foldVertices(QVector<bool>& inCover, QVector<bool>& removed,
                      QVector<int>& cover);

    /** @brief Select branching vertex (highest degree among remaining) */
    int selectBranch(const QVector<bool>& removed) const;

    /** @brief Count remaining edges */
    int countEdges(const QVector<bool>& removed) const;

    /** @brief Get degree of vertex among non-removed vertices */
    int effectiveDegree(int v, const QVector<bool>& removed) const;
};
