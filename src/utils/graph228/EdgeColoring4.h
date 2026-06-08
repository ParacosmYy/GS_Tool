/**
 * @file EdgeColoring4.h
 * @brief 边着色(Misra-Gries贪心算法+Vizing定理近优界强制) — Edge Coloring with Misra-Gries Greedy Algorithm and Vizing's Theorem Near-Optimal Bound Enforcement
 *
 * 功能: 实现图边着色，支持Misra-Gries贪心算法、
 *       Vizing定理Δ或Δ+1着色和颜色冲突检测。
 *
 * 协作: VertexColoring3(顶点着色) / GraphBFS5(广度优先搜索) / BipartiteMatch2(二分匹配)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 边着色(Misra-Gries贪心+Vizing近优界)
 */
class EdgeColoring4 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numVertices = 0;
        int numEdges = 0;
        int maxDegree = 0;
        int colorsUsed = 0;
        bool vizingOptimal = false;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief An edge in the graph */
    struct Edge {
        int u = -1;
        int v = -1;
        int color = -1;  // -1 = uncolored
    };

    explicit EdgeColoring4(QObject *parent = nullptr);
    ~EdgeColoring4() override;

    /** @brief Build graph from edge list */
    void buildGraph(int numVertices, const QVector<QPair<int, int>>& edges);

    /** @brief Color edges using Misra-Gries algorithm */
    QVector<int> color();

    /** @brief Get the color assigned to an edge */
    int edgeColor(int edgeIndex) const;

    /** @brief Verify coloring correctness */
    bool verifyColoring() const;

    /** @brief Get maximum degree of the graph */
    int maxDegree() const;

    /** @brief Check if Vizing's theorem bound is met (Δ or Δ+1) */
    bool isVizingOptimal() const;

    /** @brief Get free colors at a vertex */
    QVector<int> freeColorsAt(int vertex) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void coloringCompleted(int colorsUsed, int edges, double timeMs);

private:
    int m_n = 0;
    int m_maxDegree = 0;
    int m_colorsUsed = 0;
    QVector<Edge> m_edges;

    // Adjacency: vertex -> list of (edgeIndex, otherVertex)
    QVector<QVector<QPair<int, int>>> m_adj;

    // Color availability: vertex -> set of colors used
    QVector<QVector<bool>> m_vertexColors;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Find free color at vertex (smallest unused) */
    int findFreeColor(int vertex) const;

    /** @brief Find fan at vertex u starting from edge (u,v) */
    QVector<int> buildFan(int u, int v) const;

    /** @brief Invert maximal alternating path starting with color c */
    void invertPath(int start, int c1, int c2);

    /** @brief Rotate fan and assign color */
    void rotateFan(const QVector<int>& fan, int u, int freeColor);
};
