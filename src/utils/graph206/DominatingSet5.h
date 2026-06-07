/**
 * @file DominatingSet5.h
 * @brief 支配集(贪心+LP舍入界限+迭代关键顶点包含) — Dominating Set via Greedy with LP-Rounding Bound and Iterative Critical Vertex Inclusion
 *
 * 功能: 实现支配集算法，支持贪心启发式、
 *       LP舍入近似界限和迭代关键顶点包含策略。
 *
 * 协作: VertexCover5(顶点覆盖) / MaxClique7(最大团) / GraphColoring6(图着色)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 支配集求解器(贪心+LP舍入+关键顶点)
 */
class DominatingSet5 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalRuns = 0;
        int numVertices = 0;
        int numEdges = 0;
        int dominatingSize = 0;
        double lpBound = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit DominatingSet5(QObject *parent = nullptr);
    ~DominatingSet5() override;

    /** @brief Build graph from adjacency list */
    void setGraph(const QVector<QVector<int>>& adjList);

    /** @brief Build graph from edge list */
    void setEdges(const QVector<QPair<int, int>>& edges, int vertexCount);

    /** @brief Compute greedy dominating set */
    QVector<int> greedyDominatingSet();

    /** @brief Compute LP-relaxation lower bound */
    double lpRelaxationBound();

    /** @brief Compute dominating set with critical vertex inclusion */
    QVector<int> criticalVertexSet();

    /** @brief Verify if a set is a valid dominating set */
    bool isDominating(const QVector<int>& candidate) const;

    int vertexCount() const { return m_n; }
    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void dominatingSetFound(int size, double lpBound, double timeMs);

private:
    int m_n = 0;
    QVector<QVector<int>> m_adj;    // Adjacency list
    QVector<int> m_degree;          // Cached degrees

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Identify vertices that must be in any dominating set */
    QVector<int> findCriticalVertices() const;

    /** @brief Compute uncovered count for each vertex */
    QVector<int> computeUncovered(
        const QVector<bool>& dominated) const;
};
