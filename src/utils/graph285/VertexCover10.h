/**
 * @file VertexCover10.h
 * @brief 顶点覆盖(LP松弛舍入与最大匹配2近似有界预算优化) — Vertex Cover with LP Relaxation Rounding and Maximal Matching 2-Approximation for Bounded Budget Optimization
 *
 * 功能: 实现顶点覆盖(Vertex cover)，采用LP松弛舍入(LP relaxation rounding)
 *       与最大匹配2近似(maximal matching 2-approximation)实现有界预算优化(bounded budget optimization)。
 *
 * 协作: GraphColoring8(图着色) / MaxClique8(最大团) / MinSpanningTree8(最小生成树)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 顶点覆盖(LP松弛舍入与最大匹配2近似有界预算优化)
 */
class VertexCover10 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numVertices = 0;
        int numEdges = 0;
        int coverSize = 0;
        double lpObjective = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit VertexCover10(QObject *parent = nullptr);
    ~VertexCover10() override;

    /** @brief Set adjacency matrix (symmetric, 0/1) */
    void setGraph(const QVector<QVector<int>>& adjacency);

    /** @brief Set budget constraint (max vertices to select, 0 = unlimited) */
    void setBudget(int budget);

    /** @brief Solve via maximal matching 2-approximation */
    QVector<int> solveMatching();

    /** @brief Solve via LP relaxation rounding */
    QVector<int> solveLPRounding();

    /** @brief Solve with budget constraint (greedy on LP fractional values) */
    QVector<int> solveBudgeted();

    /** @brief Check if given vertices form a valid cover */
    bool validateCover(const QVector<int>& cover) const;

    /** @brief Get current adjacency matrix */
    QVector<QVector<int>> graph() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void coverFound(int coverSize, int vertices, int edges, double timeMs);

private:
    int m_budget = 0;
    int m_n = 0;
    QVector<QVector<int>> m_adj;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Solve LP relaxation, returns fractional vertex values */
    QVector<double> solveLP() const;
};
