/**
 * @file IndependentSet4.h
 * @brief 最大权独立集(分支约减+LP松弛界+顶点折叠) — Maximum Weight Independent Set via Branch-and-Reduce with LP Relaxation Bound and Vertex Folding
 *
 * 功能: 实现最大权独立集求解，支持分支约减(branch-and-reduce)、
 *       LP松弛界计算、顶点折叠(vertex folding)和穷举+剪枝搜索。
 *
 * 协作: MaxFlow7(最大流) / GraphColoring5(图着色) / MinSpanningTree6(最小生成树)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 最大权独立集求解器(分支约减+LP松弛界+顶点折叠)
 */
class IndependentSet4 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalSolves = 0;
        int numVertices = 0;
        int numEdges = 0;
        int setSize = 0;
        double totalWeight = 0.0;
        int branchesExplored = 0;
        int foldsApplied = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit IndependentSet4(QObject *parent = nullptr);
    ~IndependentSet4() override;

    /** @brief Set graph adjacency list and vertex weights */
    void setGraph(const QVector<QVector<int>>& adj,
                  const QVector<double>& weights);

    /** @brief Solve for maximum weight independent set */
    QVector<int> solve();

    /** @brief Get the total weight of the best solution */
    double bestWeight() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveCompleted(int setSize, double weight, double timeMs);

private:
    QVector<QVector<int>> m_adj;
    QVector<double> m_weights;

    QVector<int> m_bestSet;
    double m_bestWeight = 0.0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Branch-and-reduce recursive search */
    void branchAndReduce(QVector<int>& candidates, QVector<int>& current,
                         double currentWeight, QVector<bool>& excluded);

    /** @brief Compute LP relaxation upper bound via fractional coloring */
    double lpBound(const QVector<int>& candidates,
                   const QVector<bool>& excluded) const;

    /** @brief Apply vertex folding reduction, return true if any applied */
    bool applyFolding(QVector<int>& candidates, QVector<bool>& excluded,
                      QVector<int>& folded);

    /** @brief Check if adding vertex v to current set is valid */
    bool isIndependent(int v, const QVector<int>& current) const;

    /** @brief Degree-1 vertex folding: fold pendant vertices */
    void foldDegree1(QVector<int>& candidates, QVector<bool>& excluded,
                     QVector<int>& folded);

    /** @brief Dominated vertex removal */
    void removeDominated(QVector<int>& candidates,
                         QVector<bool>& excluded);
};
