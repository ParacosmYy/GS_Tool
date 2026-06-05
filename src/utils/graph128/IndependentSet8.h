#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief IndependentSet8 - 独立集算法第8代实现
 *
 * 提供最大独立集和最大权独立集的求解方法，
 * 支持贪心近似、分支定界精确求解及图补集转换。
 */
class IndependentSet8 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalSolveOps = 0; double avgProcessingTimeMs = 0.0; };
    explicit IndependentSet8(QObject* parent = nullptr);
    Stats stats() const { return m_stats; }
    void resetStatistics();

    /**
     * @brief 贪心法求近似最大独立集
     * @param adjacencyMatrix 图的邻接矩阵
     * @return 独立集中各顶点索引
     */
    QVector<int> greedyMIS(const QVector<QVector<int>>& adjacencyMatrix);

    /**
     * @brief 分支定界法求精确最大独立集
     * @param adjacencyMatrix 图的邻接矩阵
     * @return 最大独立集顶点索引
     */
    QVector<int> exactMIS(const QVector<QVector<int>>& adjacencyMatrix);

    /**
     * @brief 带权最大独立集求解
     * @param adjacencyMatrix 图的邻接矩阵
     * @param weights 各顶点权重
     * @return 最大权独立集顶点索引
     */
    QVector<int> maxWeightIndependentSet(const QVector<QVector<int>>& adjacencyMatrix,
                                         const QVector<double>& weights);

    /**
     * @brief 将独立集问题转换为团问题（图补集）
     * @param adjacencyMatrix 原图邻接矩阵
     * @return 补图邻接矩阵
     */
    QVector<QVector<int>> toComplementGraph(const QVector<QVector<int>>& adjacencyMatrix);

signals:
    void solveCompleted(int setSize);

private:
    Stats m_stats; double m_timeSum = 0.0;
};
