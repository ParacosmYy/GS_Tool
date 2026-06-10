/**
 * @file GraphColoring11.h
 * @brief 图着色(DSATUR饱和度排序与回溯传播色数最小化) — Graph Coloring with DSATUR Saturation Degree Ordering and Backtracking with Propagation for Chromatic Number Minimization
 *
 * 功能: 实现图着色(graph coloring)，采用DSATUR饱和度排序(DSATUR saturation degree ordering)
 *       与回溯传播(backtracking with propagation)实现色数最小化(chromatic number minimization)。
 *
 * 协作: BTree9(B树索引) / MaxFlow10(最大流) / TopologicalSort8(拓扑排序)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 图着色(DSATUR饱和度排序与回溯传播色数最小化)
 */
class GraphColoring11 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numVertices = 0;
        int numEdges = 0;
        int chromaticNumber = 0;
        int backtracks = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit GraphColoring11(QObject *parent = nullptr);
    ~GraphColoring11() override;

    /** @brief Build adjacency list from edge list (undirected graph) */
    void setGraph(int numVertices, const QVector<QPair<int, int>>& edges);

    /** @brief Set upper bound on colors to try */
    void setMaxColors(int maxColors);

    /** @brief Run DSATUR with backtracking, returns color per vertex (-1 = uncolored) */
    QVector<int> color();

    /** @brief Get chromatic number from last run */
    int chromaticNumber() const;

    /** @brief Check if current coloring is valid (no adjacent same-color) */
    bool isValidColoring(const QVector<int>& colors) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void coloringDone(int numVertices, int chromaticNumber, int backtracks, double timeMs);

private:
    int m_numVertices = 0;
    int m_maxColors = 64;

    /** @brief Adjacency list */
    QVector<QVector<int>> m_adjList;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Per-vertex DSATUR state */
    struct VertexState {
        int saturation = 0;     // number of distinct neighbor colors
        int degree = 0;         // uncolored neighbor count
        QVector<bool> usedColors;
    };

    /** @brief Select next vertex by DSATUR: max saturation, then max degree */
    int selectVertex(const QVector<VertexState>& state, const QVector<int>& colors) const;

    /** @brief Backtracking search with propagation */
    bool backtrack(QVector<int>& colors, QVector<VertexState>& state,
                   int colored, int targetColors, int& backtracks);

    /** @brief Update saturation degrees after assigning a color */
    void propagateColor(int vertex, int color, QVector<VertexState>& state,
                        const QVector<int>& colors);
};
