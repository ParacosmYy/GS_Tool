/**
 * @file EdgeColoring10.h
 * @brief 边着色(Vizing邻接分类与临界路径增广近似最优边色数) — Edge Coloring with Vizing Adjacency Classification and Critical Path Augmentation for Near-optimal Edge Chromatic Index
 *
 * 功能: 实现边着色(edge coloring)，采用Vizing邻接分类(Vizing adjacency classification)
 *       与临界路径增广(critical path augmentation)实现近似最优边色数(near-optimal edge chromatic index)。
 *
 * 协作: GraphColoring9(图着色) / Matching8(匹配) / MaxFlow7(最大流)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QMap>

/**
 * @brief 边着色(Vizing邻接分类与临界路径增广近似最优边色数)
 */
class EdgeColoring10 : public QObject {
    Q_OBJECT

public:
    /** @brief Edge representation */
    struct Edge {
        int u = -1;
        int v = -1;
        int color = -1;             // Assigned color (-1 = uncolored)
    };

    /** @brief Coloring result */
    struct ColoringResult {
        QVector<Edge> edges;
        int numColors = 0;
        int maxDegree = 0;
        bool isOptimal = false;     // True if delta colors used
        int iterations = 0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numVertices = 0;
        int numEdges = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit EdgeColoring10(QObject *parent = nullptr);
    ~EdgeColoring10() override;

    /** @brief Color edges of an undirected graph given as edge list */
    ColoringResult color(const QVector<QPair<int, int>>& edgeList, int numVertices);

    /** @brief Verify edge coloring is valid */
    bool verifyColoring(const ColoringResult& result) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void coloringDone(int edges, int colors, double timeMs);

private:
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute maximum degree of graph */
    static int maxDegree(const QVector<QPair<int, int>>& edges, int n);

    /** @brief Find missing color at vertex v */
    int missingColor(int v, const QVector<QVector<int>>& colorAt,
                     int maxColor) const;

    /** @brief Find maximal alternating path (Vizing fan) */
    QVector<int> vizingFan(int u, const Edge& uncolored,
                           const QVector<QVector<int>>& colorAt,
                           const QVector<int>& adjColor) const;

    /** @brief Rotate colors along a fan path */
    void rotateFan(QVector<Edge>& edges, const QVector<int>& fan,
                   const QVector<QVector<int>>& colorAt);

    /** @brief Find and flip a two-color alternating path */
    bool flipAlternatingPath(int start, int c1, int c2,
                             QVector<QVector<int>>& colorAt,
                             QVector<int>& freeColor) const;

    /** @brief Assign a color to an edge, updating adjacency info */
    void assignColor(int edgeIdx, int color, QVector<Edge>& edges,
                     QVector<QVector<int>>& colorAt);
};
