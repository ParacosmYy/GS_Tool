#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief GraphColoring10 - 图着色算法第10代实现
 *
 * 提供多种图着色策略，包括贪心着色、DSATUR、
 * 回溯法精确求解及色数下界估计。
 */
class GraphColoring10 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalColoringOps = 0; double avgProcessingTimeMs = 0.0; };
    explicit GraphColoring10(QObject* parent = nullptr);
    Stats stats() const { return m_stats; }
    void resetStatistics();

    /**
     * @brief 贪心法图着色
     * @param adjacencyMatrix 图的邻接矩阵
     * @return 各顶点分配的颜色编号
     */
    QVector<int> greedyColoring(const QVector<QVector<int>>& adjacencyMatrix);

    /**
     * @brief DSATUR算法着色（饱和度优先）
     * @param adjacencyMatrix 图的邻接矩阵
     * @return 各顶点分配的颜色编号
     */
    QVector<int> dsaturColoring(const QVector<QVector<int>>& adjacencyMatrix);

    /**
     * @brief 回溯法求精确色数
     * @param adjacencyMatrix 图的邻接矩阵
     * @param maxColors 最大颜色数上限
     * @return 着色方案，无解则返回空
     */
    QVector<int> exactColoring(const QVector<QVector<int>>& adjacencyMatrix, int maxColors);

    /**
     * @brief 计算色数下界（团数估计）
     * @param adjacencyMatrix 图的邻接矩阵
     * @return 色数下界值
     */
    int computeChromaticLowerBound(const QVector<QVector<int>>& adjacencyMatrix);

signals:
    void coloringCompleted(int colorCount);

private:
    Stats m_stats; double m_timeSum = 0.0;
};
