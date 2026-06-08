/**
 * @file GraphColoring7.h
 * @brief 图着色(DSATUR饱和度启发+前向检查回溯传播) — Graph Coloring with DSATUR Saturation Degree Heuristic and Backtracking with Forward-Checking Propagation
 *
 * 功能: 实现图着色算法，使用DSATUR饱和度启发选择顶点、
 *       前向检查约束传播回溯搜索最优着色方案。
 *
 * 协作: GraphMatching5(图匹配) / CommunityDetect6(社区检测) / ShortestPath4(最短路径)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 图着色(DSATUR+前向检查回溯)
 */
class GraphColoring7 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numVertices = 0;
        int numEdges = 0;
        int chromaticNumber = 0;
        int backtracks = 0;
        bool optimalFound = false;
        double avgProcessingTimeMs = 0.0;
    };

    explicit GraphColoring7(QObject *parent = nullptr);
    ~GraphColoring7() override;

    /** @brief Set graph adjacency matrix */
    void setGraph(const QVector<QVector<int>>& adjacency);

    /** @brief Set graph via edge list */
    void setGraphEdges(int numVertices, const QVector<QPair<int,int>>& edges);

    /** @brief Find optimal coloring via DSATUR + backtracking */
    QVector<int> solve(int maxColors = 0);

    /** @brief Greedy DSATUR coloring (fast approximation) */
    QVector<int> greedyDSATUR();

    /** @brief Get saturation degree of a vertex */
    int saturationDegree(int vertex) const;

    /** @brief Verify coloring is valid */
    bool isValidColoring(const QVector<int>& colors) const;

    /** @brief Count colors used */
    int countColors(const QVector<int>& colors) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void coloringCompleted(int colors, int backtracks, double timeMs);

private:
    int m_n = 0;
    QVector<QVector<int>> m_adj;
    QVector<int> m_bestColors;
    int m_chromatic = 0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Pick next vertex by DSATUR heuristic */
    int pickVertex(const QVector<int>& colors,
                   const QVector<QVector<bool>>& available) const;

    /** @brief Backtracking search with forward checking */
    bool backtrack(QVector<int>& colors, QVector<QVector<bool>>& available,
                   int colored, int maxColors);

    /** @brief Forward check: propagate constraint after coloring vertex */
    void forwardCheck(int vertex, int color,
                      QVector<QVector<bool>>& available,
                      QVector<int>& removed) const;

    /** @brief Undo forward check */
    void undoForwardCheck(int vertex,
                          QVector<QVector<bool>>& available,
                          const QVector<int>& removed) const;
};
