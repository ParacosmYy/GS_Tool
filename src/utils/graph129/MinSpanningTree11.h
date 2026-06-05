#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief MinSpanningTree11 - 最小生成树第11代实现
 *
 * 提供Kruskal和Prim两种MST算法，支持边权重矩阵输入、
 * Union-Find优化及最小生成森林计算。
 */
class MinSpanningTree11 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalSolveOps = 0; double avgProcessingTimeMs = 0.0; };
    explicit MinSpanningTree11(QObject* parent = nullptr);
    Stats stats() const { return m_stats; }
    void resetStatistics();

    /**
     * @brief Kruskal算法求最小生成树
     * @param edges 边列表 (起点, 终点, 权重)
     * @param vertexCount 顶点数
     * @return MST边列表
     */
    QVector<QPair<QPair<int, int>, double>> kruskal(
        const QVector<QPair<QPair<int, int>, double>>& edges, int vertexCount);

    /**
     * @brief Prim算法求最小生成树
     * @param adjacencyMatrix 邻接矩阵（权重为0表示不连通）
     * @return MST边列表
     */
    QVector<QPair<QPair<int, int>, double>> prim(
        const QVector<QVector<double>>& adjacencyMatrix);

    /**
     * @brief 计算最小生成森林（非连通图）
     * @param edges 边列表
     * @param vertexCount 顶点数
     * @return 各连通分量的MST边列表
     */
    QVector<QVector<QPair<QPair<int, int>, double>>> minSpanningForest(
        const QVector<QPair<QPair<int, int>, double>>& edges, int vertexCount);

    /**
     * @brief 计算MST总权重
     * @param mstEdges MST边列表
     * @return 总权重
     */
    double totalWeight(const QVector<QPair<QPair<int, int>, double>>& mstEdges) const;

signals:
    void solveCompleted(int edgeCount);

private:
    Stats m_stats; double m_timeSum = 0.0;
};
