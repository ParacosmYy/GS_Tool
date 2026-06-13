/**
 * @file EdgeColoring12.h
 * @brief 边着色(Misra-Gries边访问排序与最大匹配增广实现最多Delta+1色边着色) — Edge Coloring with Misra-Gries Edge Visit Ordering and Maximal Matching Augmentation for At Most Delta+1 Edge Colors
 *
 * 功能: 实现边着色(edge coloring)，采用Misra-Gries边访问排序(Misra-Gries edge visit ordering)
 *       与最大匹配增广(maximal matching augmentation)实现最多Delta+1色边着色(at most Delta+1 edge colors)。
 *
 * 协作: GraphColoring(图着色) / BipartiteMatching(二分匹配) / MaximumFlow(最大流)
 */
#pragma once

#include <QObject>
#include <QVector>

class EdgeColoring12 : public QObject {
    Q_OBJECT

public:
    /** @brief Graph edge */
    struct Edge {
        int from = 0;
        int to = 0;
        int color = -1;         // -1 = uncolored
    };

    /** @brief Coloring result */
    struct ColorResult {
        QVector<Edge> edges;
        int numColors = 0;
        bool valid = false;
        int maxDegree = 0;
        double elapsedMs = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalColorings = 0;
        int maxColorsUsed = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit EdgeColoring12(QObject *parent = nullptr);
    ~EdgeColoring12() override;

    /** @brief Build adjacency from edge list */
    void setGraph(int numVertices, const QVector<Edge>& edges);

    /** @brief Color edges using Misra-Gries algorithm */
    ColorResult color();

    /** @brief Verify edge coloring is proper (no adjacent edges share color) */
    bool verify(const QVector<Edge>& colored) const;

    /** @brief Get maximum vertex degree Delta */
    int maxDegree() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void coloringDone(int numColors, int numEdges, double timeMs);

private:
    int m_numVertices = 0;
    QVector<Edge> m_edges;
    Stats m_stats;
    double m_timeSum = 0.0;

    // Adjacency: vertex -> list of edge indices
    QVector<QVector<int>> m_adj;

    /** @brief Find free color at vertex (not used by incident edges) */
    int freeColor(int vertex, const QVector<int>& edgeColors) const;

    /** @brief Find maximal fan at vertex u starting from edge (u, v) */
    QVector<int> buildFan(int u, int v, const QVector<int>& edgeColors) const;

    /** @brief Find missing color at vertex */
    int missingColor(int vertex, const QVector<int>& edgeColors) const;

    /** @brief Invert CD-path (swap colors c and d along alternating path) */
    void invertPath(int start, int c, int d, QVector<int>& edgeColors);

    /** @brief Rotate fan and assign color */
    void rotateFan(const QVector<int>& fan, int freeCol, QVector<int>& edgeColors);
};
