#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief DominatingSet9 - 支配集算法第9代实现
 *
 * 提供最小支配集和最小连通支配集的求解方法，
 * 支持贪心近似算法及精确求解。
 */
class DominatingSet9 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalSolveOps = 0; double avgProcessingTimeMs = 0.0; };
    explicit DominatingSet9(QObject* parent = nullptr);
    Stats stats() const { return m_stats; }
    void resetStatistics();

    /**
     * @brief 贪心法求近似最小支配集
     * @param adjacencyMatrix 图的邻接矩阵
     * @return 支配集顶点索引
     */
    QVector<int> greedyMDS(const QVector<QVector<int>>& adjacencyMatrix);

    /**
     * @brief 求最小连通支配集
     * @param adjacencyMatrix 图的邻接矩阵
     * @return 连通支配集顶点索引
     */
    QVector<int> connectedMDS(const QVector<QVector<int>>& adjacencyMatrix);

    /**
     * @brief 验证给定集合是否为支配集
     * @param candidateSet 候选集合
     * @param adjacencyMatrix 图的邻接矩阵
     * @return 是否为有效支配集
     */
    bool isDominatingSet(const QVector<int>& candidateSet,
                         const QVector<QVector<int>>& adjacencyMatrix) const;

    /**
     * @brief 计算支配集的近似比
     * @param greedySize 贪心解大小
     * @param optimalSize 最优解大小
     * @return 近似比
     */
    double approximationRatio(int greedySize, int optimalSize) const;

signals:
    void solveCompleted(int setSize);

private:
    Stats m_stats; double m_timeSum = 0.0;
};
