/**
 * @file VertexCover9.h
 * @brief 顶点覆盖(LP松弛舍入+皇冠分解核化固定参数可解) — Vertex Cover with LP Relaxation Rounding and Kernelization via Crown Decomposition for Fixed-parameter Tractability
 *
 * 功能: 实现顶点覆盖(Vertex Cover)算法，采用LP松弛舍入(LP relaxation
 *       rounding)与皇冠分解核化(crown decomposition kernelization)
 *       实现固定参数可解(fixed-parameter tractable)求解。
 *
 * 协作: MaxFlow7(最大流) / MinSpanTree6(最小生成树) / GraphColoring5(图着色)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 顶点覆盖(LP松弛舍入+皇冠分解核化固定参数可解)
 */
class VertexCover9 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numVertices = 0;
        int numEdges = 0;
        int coverSize = 0;
        int kernelSize = 0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Graph edge */
    struct Edge {
        int u = -1;
        int v = -1;
    };

    explicit VertexCover9(QObject *parent = nullptr);
    ~VertexCover9() override;

    /** @brief Set parameter k for FPT search */
    void setParameterK(int k);

    /** @brief Build graph from edge list */
    void setGraph(int numVertices, const QVector<Edge>& edges);

    /** @brief Solve via LP relaxation + rounding */
    QVector<int> solveLP();

    /** @brief Solve via kernelization + bounded search */
    QVector<int> solveKernelized();

    /** @brief Find crown decomposition for kernelization */
    bool findCrown(QVector<int>& crown, QVector<int>& head, QVector<int>& body) const;

    /** @brief Get vertex cover size */
    int coverSize() const;

    /** @brief Verify a candidate cover */
    bool verifyCover(const QVector<int>& cover) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void coverFound(int size, int kernelVertices, double timeMs);

private:
    int m_paramK = 10;
    int m_n = 0;
    QVector<Edge> m_edges;
    QVector<QVector<int>> m_adj;   // Adjacency list

    QVector<int> m_cover;
    double m_lpBound = 0.0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build adjacency list from edge list */
    void buildAdjList();

    /** @brief LP relaxation: fractional vertex cover via dual (matching) */
    double computeLPBound();

    /** @brief Bounded search tree for FPT */
    bool boundedSearch(QVector<bool>& inCover, int remaining, int edgeIdx);

    /** @brief Find maximal matching */
    QVector<Edge> maximalMatching() const;

    /** @brief Find augmenting path for bipartite matching */
    bool augment(int u, QVector<int>& matchTo, QVector<bool>& visited) const;
};
