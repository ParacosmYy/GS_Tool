/**
 * @file VertexCover11.h
 * @brief 顶点覆盖(原始对偶方案与半积分松弛的2近似加权算法) — Vertex Cover with Primal-dual Scheme and Half-integrality Relaxation for 2-approximation Weighted Vertex Cover
 *
 * 功能: 实现顶点覆盖(Vertex cover)，采用原始对偶方案(primal-dual scheme)
 *       与半积分松弛(half-integrality relaxation)实现2近似加权顶点覆盖(2-approximation weighted vertex cover)。
 *
 * 协作: MaxMatching12(最大匹配) / GraphColoring13(图着色) / MinSpanTree14(最小生成树)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 顶点覆盖(原始对偶方案与半积分松弛)
 */
class VertexCover11 : public QObject {
    Q_OBJECT

public:
    /** @brief Graph edge */
    struct Edge {
        int u = -1;
        int v = -1;
        double weight = 1.0;       // Edge weight (unused for vertex cover)
    };

    /** @brief Vertex cover result */
    struct CoverResult {
        QVector<int> coverVertices;    // Selected vertex indices
        double totalWeight = 0.0;
        int numCovered = 0;
        int totalEdges = 0;
        double approximationRatio = 2.0;
    };

    /** @brief LP relaxation solution (half-integrality) */
    struct LPSolution {
        QVector<double> x;             // LP variables [0, 0.5, 1]
        double lpObjective = 0.0;
        bool halfIntegral = true;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numVertices = 0;
        int numEdges = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit VertexCover11(QObject *parent = nullptr);
    ~VertexCover11() override;

    /** @brief Build graph from edge list with vertex weights */
    void buildGraph(int numVertices, const QVector<Edge>& edges,
                    const QVector<double>& vertexWeights);

    /** @brief Solve weighted vertex cover via primal-dual 2-approximation */
    CoverResult solvePrimalDual();

    /** @brief Solve LP relaxation and round (half-integrality) */
    CoverResult solveLPRounding();

    /** @brief Get LP relaxation solution */
    LPSolution solveLP() const;

    /** @brief Verify cover is valid */
    bool verifyCover(const QVector<int>& cover) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveDone(int vertices, int edges, double weight, double timeMs);

private:
    int m_n = 0;                     // Number of vertices
    Stats m_stats;
    double m_timeSum = 0.0;

    QVector<Edge> m_edges;
    QVector<double> m_vertexWeights;
    QVector<QVector<int>> m_adj;     // Adjacency list (edge indices)

    /** @brief Primal-dual: raise dual variables, select tight vertices */
    CoverResult runPrimalDual();

    /** @brief LP rounding: solve LP, round {0.5} up to 1 */
    CoverResult runLPRounding();

    /** @brief Compute adjacency list from edge list */
    void buildAdjacency();
};
