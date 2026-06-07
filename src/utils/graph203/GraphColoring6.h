/**
 * @file GraphColoring6.h
 * @brief 图着色(DSATUR饱和度+回溯+色数下界) — Graph Coloring via DSATUR with Backtracking and Chromatic Number Lower Bound
 *
 * 功能: 实现图着色算法，支持DSATUR(饱和度)启发式排序、
 *       回溯搜索、色数下界估计和冲突检测。
 *
 * 协作: GraphBridges4(桥检测) / TopologicalSort5(拓扑排序) / ShortestPath8(最短路径)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 图着色器(DSATUR+回溯+色数下界)
 */
class GraphColoring6 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalRuns = 0;
        int numVertices = 0;
        int numEdges = 0;
        int colorsUsed = 0;
        int backtrackSteps = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit GraphColoring6(QObject *parent = nullptr);
    ~GraphColoring6() override;

    /** @brief Build graph from adjacency lists (0-indexed) */
    void setGraph(const QVector<QVector<int>>& adjacency);

    /** @brief Set maximum colors to try (0 = auto) */
    void setMaxColors(int max);

    /** @brief Run DSATUR coloring with backtracking */
    QVector<int> color();

    /** @brief Compute chromatic number lower bound (clique number) */
    int chromaticLowerBound() const;

    /** @brief Verify coloring is valid (no adjacent same-color) */
    bool verifyColoring(const QVector<int>& coloring) const;

    /** @brief Greedy coloring (Welsh-Powell) for comparison */
    QVector<int> greedyColor() const;

    int colorsUsed() const { return m_colorsUsed; }
    int vertexCount() const { return m_adj.size(); }
    int edgeCount() const { return m_edgeCount; }
    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void coloringCompleted(int vertices, int colors, int backtracks, double timeMs);

private:
    QVector<QVector<int>> m_adj;
    int m_maxColors = 0;
    int m_edgeCount = 0;
    int m_colorsUsed = 0;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief DSATUR: pick vertex with highest saturation degree */
    int pickDsaturVertex(const QVector<int>& colors,
                         const QVector<int>& saturation) const;

    /** @brief Recursive backtracking helper */
    bool backtrack(QVector<int>& colors, int idx, int maxC, int& steps,
                   const QVector<int>& order);

    /** @brief Count edges in graph */
    int countEdges() const;

    /** @brief Greedy max clique lower bound */
    int maxCliqueSize() const;
};
