/**
 * @file EdgeColoring6.h
 * @brief 边着色(Misra-Gries边访问序+扇形旋转至多Δ+1色) — Edge Coloring with Misra-Gries Edge Visit Order and Fan Rotation for at Most Delta+1 Colors
 *
 * 功能: 实现Misra-Gries边着色算法(Misra-Gries edge coloring)，通过特定边访问
 *       序(edge visit order)和扇形旋转(fan rotation)操作，使用至多Δ+1种颜色
 *       (delta+1 colors)完成图的合法边着色(legal edge coloring)。
 *
 * 协作: BipartiteMatch8(二部图匹配) / EulerTour7(欧拉回路) / MaxFlow6(最大流)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 边着色(Misra-Gries边访问序+扇形旋转至多Δ+1色)
 */
class EdgeColoring6 : public QObject {
    Q_OBJECT

public:
    /** @brief Edge representation */
    struct Edge {
        int u = -1;
        int v = -1;
        int color = -1;  // -1 = uncolored
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numVertices = 0;
        int numEdges = 0;
        int maxDegree = 0;
        int colorsUsed = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit EdgeColoring6(QObject *parent = nullptr);
    ~EdgeColoring6() override;

    /** @brief Set graph edges as pairs of vertices */
    void setEdges(const QVector<QPair<int, int>>& edges, int numVertices);

    /** @brief Run Misra-Gries edge coloring */
    QVector<Edge> color();

    /** @brief Get maximum degree of current graph */
    int maxDegree() const;

    /** @brief Verify coloring is valid (no adjacent edges share color) */
    bool verifyColoring(const QVector<Edge>& colored) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void coloringCompleted(int colorsUsed, int maxDeg, double timeMs);
    void edgeColored(int edgeIdx, int color);

private:
    int m_n = 0;  // number of vertices
    QVector<Edge> m_edges;
    QVector<QVector<int>> m_adj;  // adjacency list (edge indices)

    // Color tracking: m_vertexColor[v][c] = edge index, -1 if free
    QVector<QVector<int>> m_vertexColor;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Find free color at vertex v (not used by any incident edge) */
    int freeColor(int v) const;

    /** @brief Find the neighbor of v via the edge of given color */
    int neighborOnColor(int v, int c) const;

    /** @brief Build maximal fan from free vertex pivot */
    QVector<int> buildFan(int pivot, int edgeIdx) const;

    /** @brief Rotate colors along a fan */
    void rotateFan(const QVector<int>& fan, int cdash);

    /** @brief Invert maximal alternating path from vertex v on colors c1, c2 */
    void invertPath(int v, int c1, int c2);

    /** @brief Build adjacency from edge list */
    void buildAdjacency();
};
