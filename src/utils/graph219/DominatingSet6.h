/**
 * @file DominatingSet6.h
 * @brief 支配集(LP舍入+随机化舍入+迭代精炼) — Dominating Set via LP-Rounding with Randomized Rounding and Iterative Refinement
 *
 * 功能: 实现基于LP舍入的支配集算法，支持随机化舍入、
 *       迭代精炼和近似比保证。
 *
 * 协作: GraphColor5(图着色) / MaxClique4(最大团) / MinSpanTree3(最小生成树)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 支配集(LP舍入+随机化舍入+迭代精炼)
 */
class DominatingSet6 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalSolves = 0;
        int numVertices = 0;
        int dominatingSetSize = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit DominatingSet6(QObject *parent = nullptr);
    ~DominatingSet6() override;

    void setMaxIterations(int iter);
    void setRoundTrials(int trials);

    /** @brief Solve dominating set: adjacency list input */
    QVector<int> solve(const QVector<QVector<int>>& adjList);

    /** @brief Solve LP relaxation to get fractional solution */
    QVector<double> solveLP(const QVector<QVector<int>>& adjList) const;

    /** @brief Randomized rounding of fractional LP solution */
    QVector<int> randomizedRound(const QVector<double>& fractional,
                                  const QVector<QVector<int>>& adjList) const;

    /** @brief Iterative refinement: remove redundant vertices */
    QVector<int> refine(const QVector<int>& dominating,
                         const QVector<QVector<int>>& adjList) const;

    /** @brief Check if set is a valid dominating set */
    bool isDominating(const QVector<int>& vertices,
                       const QVector<QVector<int>>& adjList) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solved(int setSize, double timeMs);

private:
    int m_maxIter = 100;
    int m_roundTrials = 20;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Count uncovered vertices */
    int countUncovered(const QVector<bool>& covered) const;
};
