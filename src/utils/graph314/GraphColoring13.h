/**
 * @file GraphColoring13.h
 * @brief 图着色(递归最大优先与饱和度排序实现紧界顺序顶点着色) — Graph Coloring with Recursive Largest First and Saturation Degree Ordering for Sequential Vertex Coloring with Tight Bounds
 *
 * 功能: 实现图着色(graph coloring)，采用递归最大优先(recursive largest first)
 *       与饱和度排序(saturation degree ordering)实现紧界顺序顶点着色(sequential vertex coloring with tight bounds)。
 *
 * 协作: GraphIsomorphism(图同构) / MaximumClique(最大团) / GraphPartitioning(图划分)
 */
#pragma once

#include <QObject>
#include <QVector>

class GraphColoring13 : public QObject {
    Q_OBJECT

public:
    /** @brief Coloring result with quality metrics */
    struct ColoringResult {
        QVector<int> colors;          // color assignment per vertex (-1 = uncolored)
        int numColors = 0;            // chromatic number upper bound
        bool isValid = false;         // conflict-free verification
        double density = 0.0;         // graph density
    };

    /** @brief Vertex ordering info for DSATUR */
    struct VertexInfo {
        int index = 0;
        int saturation = 0;           // number of distinct neighbor colors
        int degree = 0;               // uncolored neighbor count
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalColorings = 0;
        int numVertices = 0;
        int numEdges = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit GraphColoring13(QObject *parent = nullptr);
    ~GraphColoring13() override;

    /** @brief Build adjacency list from edge list (undirected) */
    void setGraph(int numVertices, const QVector<QPair<int, int>>& edges);

    /** @brief Color using DSATUR (saturation degree ordering) */
    ColoringResult colorDSATUR();

    /** @brief Color using Recursive Largest First */
    ColoringResult colorRLF();

    /** @brief Verify no two adjacent vertices share the same color */
    bool verifyColoring(const QVector<int>& colors) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void coloringDone(int vertices, int colors, double timeMs);

private:
    int m_numVertices = 0;
    QVector<QVector<int>> m_adj;      // adjacency list
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Select next vertex by DSATUR rule (max saturation, tie-break degree) */
    int selectDSATURVertex(const QVector<int>& colors, const QVector<int>& saturation) const;

    /** @brief Find lowest available color for a vertex */
    int findLowestColor(int vertex, const QVector<int>& colors) const;

    /** @brief Update saturation degrees after coloring a vertex */
    void updateSaturation(int vertex, const QVector<int>& colors,
                          QVector<int>& saturation) const;

    /** @brief RLF: find uncolored vertices adjacent to current color class */
    QVector<int> findUncoloredNonAdjacent(const QVector<int>& colors, int color,
                                           const QVector<int>& candidates) const;
};
