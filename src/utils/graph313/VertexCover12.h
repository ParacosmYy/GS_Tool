/**
 * @file VertexCover12.h
 * @brief 顶点覆盖(二部图匹配归约与LP松弛舍入实现2近似最小权重顶点覆盖) — Vertex Cover with Bipartite Matching Reduction and LP Relaxation Rounding for 2-Approximate Minimum Weight Vertex Cover
 *
 * 功能: 实现顶点覆盖(Vertex cover)，采用二部图匹配归约(bipartite matching reduction)
 *       与LP松弛舍入(LP relaxation rounding)实现2近似最小权重顶点覆盖(2-approximate minimum weight vertex cover)。
 *
 * 协作: MaxFlow(最大流) / HopcroftKarp(二部匹配) / ApproxSetCover(近似集合覆盖)
 */
#pragma once

#include <QObject>
#include <QVector>

class VertexCover12 : public QObject {
    Q_OBJECT

public:
    /** @brief Edge in the graph */
    struct Edge {
        int u = -1;
        int v = -1;
        double weight = 1.0;
    };

    /** @brief Vertex cover solution */
    struct CoverResult {
        QVector<int> coverVertices;    // selected vertex indices
        double totalWeight = 0.0;
        bool isApproximate = true;
        double approximationRatio = 2.0;
    };

    /** @brief LP relaxation solution (fractional) */
    struct LPSolution {
        QVector<double> fractionalValues;  // x_v in [0, 1]
        double objectiveValue = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalSolves = 0;
        int numVertices = 0;
        int numEdges = 0;
        int coverSize = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit VertexCover12(QObject *parent = nullptr);
    ~VertexCover12() override;

    /** @brief Build graph from edge list */
    void buildGraph(int numVertices, const QVector<Edge>& edges);

    /** @brief Set vertex weights */
    void setVertexWeights(const QVector<double>& weights);

    /** @brief Solve 2-approximate minimum weight vertex cover via LP relaxation rounding */
    CoverResult solveLP();

    /** @brief Solve via bipartite matching reduction (for bipartite graphs) */
    CoverResult solveBipartite();

    /** @brief Get LP relaxation fractional solution */
    LPSolution lpRelaxation() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveDone(int coverSize, double totalWeight, double timeMs);

private:
    int m_n = 0;
    QVector<Edge> m_edges;
    QVector<double> m_weights;
    QVector<QVector<int>> m_adj;    // adjacency list
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Check if graph is bipartite via BFS coloring */
    bool isBipartite(QVector<int>& coloring) const;

    /** @brief Solve LP relaxation via deterministic primal-dual scheme */
    LPSolution solveLPRelaxation() const;

    /** @brief Round fractional LP solution to integral cover */
    CoverResult roundLPSolution(const LPSolution& lp) const;

    /** @brief Maximum matching via augmenting paths on bipartite graph */
    QVector<int> maximumMatching(const QVector<int>& coloring) const;

    /** @brief BFS augmenting path for matching */
    bool augmentingPath(int u, QVector<int>& matchU, QVector<int>& matchV,
                        QVector<bool>& visited) const;

    /** @brief Konig's theorem: extract vertex cover from matching */
    CoverResult konigCover(const QVector<int>& matching, const QVector<int>& coloring) const;
};
