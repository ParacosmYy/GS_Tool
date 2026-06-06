/**
 * @file DominatingSet4.h
 * @brief 最小支配集(贪心近似+迭代改进+顶点覆盖评分) — Minimum Dominating Set via Greedy Approximation with Iterative Improvement and Vertex Coverage Scoring
 *
 * 功能: 实现最小支配集算法，支持贪心近似构造、迭代改进优化、
 *       顶点覆盖评分和连通支配集约束。
 *
 * 协作: MaxClique4(最大团) / GraphColoring4(图着色) / MinSpanTree4(最小生成树)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 最小支配集求解器
 */
class DominatingSet4 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalRuns = 0;
        int numVertices = 0;
        int dominatingsetSize = 0;
        double coverageRatio = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit DominatingSet4(QObject *parent = nullptr);
    ~DominatingSet4() override;

    /** @brief 设置邻接表(每行是顶点i的邻居列表) */
    void setGraph(const QVector<QVector<int>>& adjacency);

    /** @brief 求解最小支配集(贪心+迭代改进) */
    QVector<int> solve();

    /** @brief 验证支配集正确性 */
    bool validate(const QVector<int>& dominatingSet) const;

    /** @brief 计算顶点覆盖分数 */
    double vertexCoverageScore(int vertex) const;

    /** @brief 获取所有顶点的覆盖分数 */
    QVector<QPair<int, double>> allCoverageScores() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveCompleted(int setSize, double coverage);
    void improvementStep(int iteration, int setSize);

private:
    int m_numVertices = 0;
    QVector<QVector<int>> m_adj;     ///< Adjacency list
    QVector<int> m_dominatingSet;

    Stats m_stats;
    double m_timeSum = 0.0;
};
