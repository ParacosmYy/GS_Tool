/**
 * @file EdgeColoring2.h
 * @brief 边着色(Misra-Gries+Vizing邻接表+类增广) — Edge Coloring via Misra-Gries with Vizing's Adjacency List and Class Augmentation
 *
 * 功能: 实现图边着色算法，支持Misra-Gries算法、
 *       Vizing邻接表颜色管理和路径类增广。
 *
 * 协作: GraphColoring2(顶点着色) / BipartiteMatch3(二分匹配) / ChromaticPoly2(色多项式)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 边着色(Misra-Gries+Vizing邻接表+类增广)
 */
class EdgeColoring2 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalColorings = 0;
        int numVertices = 0;
        int numEdges = 0;
        int colorsUsed = 0;
        int maxDegree = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit EdgeColoring2(QObject *parent = nullptr);
    ~EdgeColoring2() override;

    /** @brief Color edges using Misra-Gries algorithm */
    QVector<int> color(const QVector<QPair<int, int>>& edges, int numVertices);

    /** @brief Get the color assigned to each edge */
    int edgeColor(int u, int v) const;

    /** @brief Verify edge coloring is valid */
    bool isValid(const QVector<QPair<int, int>>& edges,
                 const QVector<int>& colors) const;

    /** @brief Compute maximum degree of graph */
    int maxDegree(int numVertices,
                  const QVector<QPair<int, int>>& edges) const;

    int colorsUsed() const { return m_colorsUsed; }
    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void coloringCompleted(int vertices, int edges, int colors, double timeMs);

private:
    int m_colorsUsed = 0;

    // Adjacency list: vertex -> list of (neighbor, color)
    QVector<QVector<QPair<int, int>>> m_adj;

    // edgeColor[u][v] = color of edge (u,v), -1 if uncolored
    QVector<QVector<int>> m_edgeColor;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Find a free color at vertex v */
    int freeColor(int v, int maxCol) const;

    /** @brief Find maximal fan starting at x through uncolored edge */
    QVector<int> buildFan(int x, int r, int maxCol) const;

    /** @brief Find alternating path and invert colors */
    void flipPath(int x, int cd, int cf, int maxCol);

    /** @brief Rotate fan and assign color */
    void rotateFan(int x, const QVector<int>& fan, int cd);
};
