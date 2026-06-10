/**
 * @file EdgeColoring8.h
 * @brief 边着色(Misra-Gries贪心赋色与Vizing增广近优色指) — Edge Coloring with Misra-Gries Greedy Assignment and Vizing Augmentation for Near-Optimal Chromatic Index
 *
 * 功能: 实现边着色(Edge coloring)，采用Misra-Gries贪心赋色(Misra-Gries greedy assignment)
 *       与Vizing增广(Vizing augmentation)实现近优色指(near-optimal chromatic index)。
 *
 * 协作: GraphColoring7(图着色) / BipartiteMatching5(二分匹配) / ChromaticPolynomial6(色多项式)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 边着色(Misra-Gries贪心赋色与Vizing增广近优色指)
 */
class EdgeColoring8 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numVertices = 0;
        int numEdges = 0;
        int maxDegree = 0;
        int numColors = 0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief An edge in the graph */
    struct Edge {
        int u = -1;
        int v = -1;
        int color = -1;
    };

    explicit EdgeColoring8(QObject *parent = nullptr);
    ~EdgeColoring8() override;

    /** @brief Build graph from edge list */
    void setGraph(int numVertices, const QVector<QPair<int, int>>& edges);

    /** @brief Run Misra-Gries edge coloring */
    QVector<Edge> color();

    /** @brief Get the chromatic index (number of colors used) */
    int chromaticIndex() const;

    /** @brief Verify the coloring is valid (no adjacent edges share color) */
    bool verifyColoring() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void coloringCompleted(int numColors, int maxDegree, double timeMs);

private:
    int m_numVertices = 0;
    QVector<Edge> m_edges;
    int m_chromaticIndex = 0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Get maximum vertex degree */
    int maxDegree() const;

    /** @brief Find a free color for a vertex at given edge index */
    int freeColor(int vertex, const QVector<QVector<int>>& edgeColors) const;

    /** @brief Find maximal fan from vertex u at uncolored edge */
    QVector<int> buildFan(int u, int v,
                           const QVector<QVector<int>>& adjColors) const;

    /** @brief Invert a color along an alternating path */
    void invertPath(int start, int c1, int c2,
                    QVector<QVector<int>>& adjColors,
                    QVector<int>& vertexColor1,
                    QVector<int>& vertexColor2) const;
};
