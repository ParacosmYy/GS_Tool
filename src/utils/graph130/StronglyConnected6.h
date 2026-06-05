#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief StronglyConnected6 - 强连通分量第6代实现
 *
 * 提供Kosaraju和Tarjan两种SCC算法，支持有向图的
 * 强连通分量分解、缩图及拓扑排序。
 */
class StronglyConnected6 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalSolveOps = 0; double avgProcessingTimeMs = 0.0; };
    explicit StronglyConnected6(QObject* parent = nullptr);
    Stats stats() const { return m_stats; }
    void resetStatistics();

    /**
     * @brief Kosaraju算法求强连通分量
     * @param adjacencyList 有向图邻接表
     * @return 各顶点所属的SCC编号
     */
    QVector<int> kosaraju(const QVector<QVector<int>>& adjacencyList);

    /**
     * @brief Tarjan算法求强连通分量
     * @param adjacencyList 有向图邻接表
     * @return 各顶点所属的SCC编号
     */
    QVector<int> tarjan(const QVector<QVector<int>>& adjacencyList);

    /**
     * @brief 构建SCC缩图（将每个SCC缩为一个超级顶点）
     * @param adjacencyList 原图邻接表
     * @param sccLabels 各顶点的SCC编号
     * @return 缩图后的DAG邻接表
     */
    QVector<QVector<int>> condensationGraph(const QVector<QVector<int>>& adjacencyList,
                                            const QVector<int>& sccLabels);

    /**
     * @brief 对SCC缩图进行拓扑排序
     * @param dag 缩图DAG邻接表
     * @return 拓扑排序结果
     */
    QVector<int> topologicalSort(const QVector<QVector<int>>& dag);

signals:
    void solveCompleted(int sccCount);

private:
    Stats m_stats; double m_timeSum = 0.0;
};
