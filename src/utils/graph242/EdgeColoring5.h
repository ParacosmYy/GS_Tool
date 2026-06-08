/**
 * @file EdgeColoring5.h
 * @brief 边着色(三次平面图Tait算法+一般图Kempe链换色) — Edge Coloring with Tait's Algorithm for Cubic Planar Graphs and Kempe Chain Recoloring for General Graphs
 *
 * 功能: 实现图边着色算法，对三次平面图(cubic planar graph)使用Tait算法，
 *       对一般图使用Kempe链换色(Kempe chain recoloring)贪心策略。
 *
 * 协作: GraphBipartite3(二分图) / GraphMatching4(图匹配) / ChromaticPolynomial2(着色多项式)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 边着色(Tait算法+Kempe链换色)
 */
class EdgeColoring5 : public QObject {
    Q_OBJECT

public:
    /** @brief Edge representation */
    struct Edge {
        int from = 0;
        int to = 0;
        int color = -1;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numVertices = 0;
        int numEdges = 0;
        int numColors = 0;
        bool isCubicPlanar = false;
        double avgProcessingTimeMs = 0.0;
    };

    explicit EdgeColoring5(QObject *parent = nullptr);
    ~EdgeColoring5() override;

    /** @brief Color edges of a graph given by adjacency list */
    QVector<Edge> color(const QVector<QVector<int>>& adjacency);

    /** @brief Tait's algorithm for cubic planar graphs (3-regular) */
    QVector<Edge> taitColoring(const QVector<QVector<int>>& adjacency);

    /** @brief Kempe chain recoloring greedy approach */
    QVector<Edge> kempeColoring(const QVector<QVector<int>>& adjacency);

    /** @brief Check if graph is cubic (3-regular) */
    bool isCubic(const QVector<QVector<int>>& adjacency) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void coloringCompleted(int edges, int colors, double timeMs);

private:
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build edge list from adjacency */
    QVector<Edge> buildEdgeList(const QVector<QVector<int>>& adjacency) const;

    /** @brief Find Kempe chain (alternating color path) */
    QVector<int> findKempeChain(int startEdge, int color1, int color2,
                                 const QVector<Edge>& edges,
                                 const QVector<QVector<int>>& adj) const;

    /** @brief Swap colors along a Kempe chain */
    void swapKempeChain(QVector<int>& chain, int color1, int color2,
                          QVector<Edge>& edges);
};
