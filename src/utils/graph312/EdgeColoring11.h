/**
 * @file EdgeColoring11.h
 * @brief 边着色(Tashkinov树增广与Vizing扇扩展实现第1类/第2类图分类) — Edge Coloring with Tashkinov Tree Augmentation and Vizing Fan Extension for Classifying Class-1 and Class-2 Graphs
 *
 * 功能: 实现边着色(edge coloring)，采用Tashkinov树增广(Tashkinov tree augmentation)
 *       与Vizing扇扩展(Vizing fan extension)实现第1类/第2类图分类(classifying class-1 and class-2 graphs)。
 *
 * 协作: GraphColoring10(顶点着色) / Matching(匹配) / ChromaticIndex(色指数)
 */
#pragma once

#include <QObject>
#include <QVector>

class EdgeColoring11 : public QObject {
    Q_OBJECT

public:
    /** @brief Edge representation */
    struct Edge {
        int u = -1;
        int v = -1;
        int color = -1;
    };

    /** @brief Coloring result */
    struct ColoringResult {
        QVector<Edge> edges;
        int numColors = 0;
        int maxDegree = 0;
        bool isClass1 = false;  // true if chromatic index = max degree (class-1)
        bool isValid = false;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalColorings = 0;
        int numVertices = 0;
        int numEdges = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit EdgeColoring11(QObject *parent = nullptr);
    ~EdgeColoring11() override;

    /** @brief Color edges of an undirected graph given as edge list */
    ColoringResult color(int numVertices, const QVector<QPair<int, int>>& edgeList);

    /** @brief Verify edge coloring is proper (no two adjacent edges share a color) */
    bool verifyColoring(const QVector<Edge>& edges) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void coloringDone(int edges, int colors, double timeMs);

private:
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Find a free color at vertex v (not used by any incident edge) */
    int findFreeColor(int v, int maxColor,
                       const QVector<QVector<int>>& vertexEdgeColors) const;

    /** @brief Build Vizing fan from vertex u centered at edge (u,v) */
    QVector<int> vizingFan(int u, int v, const QVector<QVector<int>>& adjColors,
                            const QVector<QVector<int>>& vertexEdgeColors,
                            int maxColor) const;

    /** @brief Tashkinov tree augmentation for class-2 graphs */
    bool tashkinovAugment(QVector<Edge>& edges, int edgeIdx,
                           QVector<QVector<int>>& vertexEdgeColors,
                           int maxColor, int numVertices);

    /** @brief Rotate colors along a Vizing fan path */
    void rotateFan(QVector<Edge>& edges, const QVector<int>& fan,
                    int u, int freeColor,
                    QVector<QVector<int>>& vertexEdgeColors);

    /** @brief Find alternating path for Kempe chain */
    QVector<int> kempeChain(int startVertex, int c1, int c2,
                             const QVector<QVector<int>>& adjColors,
                             int numVertices) const;
};
