/**
 * @file GraphColoring9.h
 * @brief 图着色(DSatur饱和度排序+前向检查回溯搜索) — Graph Coloring with DSatur Saturation Degree Ordering and Backtracking with Forward Checking
 *
 * 功能: 实现图着色(Graph coloring)算法，使用DSatur饱和度排序(saturation degree
 *       ordering)动态选择最高饱和度未着色顶点，结合前向检查回溯(forward checking
 *       backtracking)进行剪枝搜索，最小化使用颜色数。
 *
 * 协作: TopologicalSort6(拓扑排序) / Dijkstra12(最短路径) / AStar9(A*搜索)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 图着色(DSatur饱和度排序+前向检查回溯搜索)
 */
class GraphColoring9 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numVertices = 0;
        int numEdges = 0;
        int colorsUsed = 0;
        int backtracks = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit GraphColoring9(QObject *parent = nullptr);
    ~GraphColoring9() override;

    /** @brief Set max colors allowed (0 = auto-detect minimum) */
    void setMaxColors(int max);

    /** @brief Build graph from adjacency list */
    void setGraph(const QVector<QVector<int>>& adjacency);

    /** @brief Build graph from edge list (pairs of vertex indices) */
    void setEdges(int numVertices, const QVector<QPair<int, int>>& edges);

    /** @brief Run DSatur coloring, returns color assignment per vertex */
    QVector<int> color();

    /** @brief Verify coloring is valid */
    bool isValid() const;

    /** @brief Get chromatic number (colors actually used) */
    int chromaticNumber() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void coloringCompleted(int colorsUsed, int backtracks, double timeMs);

private:
    int m_maxColors = 0;
    int m_numVertices = 0;
    QVector<QVector<int>> m_adj;   // adjacency list
    QVector<int> m_colors;         // color assignment (-1 = uncolored)

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief DSatur greedy coloring for initial upper bound */
    int dsaturGreedy();

    /** @brief Backtracking with forward checking */
    bool backtrack(int vertex, int maxC, QVector<QVector<bool>>& domains);

    /** @brief Select next vertex by DSatur ordering */
    int selectVertex(const QVector<QVector<bool>>& domains) const;

    /** @brief Count distinct colors in neighbor set */
    int saturationDegree(int vertex) const;
};
