/**
 * @file TopologicalSort.h
 * @brief 拓扑排序(Kahn算法+环检测) — Topological Sort with Kahn's Algorithm and Cycle Detection
 *
 * 功能: 实现有向无环图的拓扑排序，基于Kahn算法(BFS入度法)，
 *       支持环检测、多解优先级排序和依赖关系分析。
 *
 * 协作: Dijkstra9(最短路径) / BellmanFord4(负权路径) / StronglyConnected(强连通分量)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 拓扑排序器
 */
class TopologicalSort : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalSorts = 0;           ///< 累计排序次数
        int lastNodeCount = 0;            ///< 最近节点数
        int lastEdgeCount = 0;            ///< 最近边数
        bool lastHadCycle = false;        ///< 最近是否有环
        double avgProcessingTimeMs = 0.0; ///< 平均耗时(ms)
    };

    explicit TopologicalSort(QObject *parent = nullptr);
    ~TopologicalSort() override;

    /**
     * @brief Kahn算法拓扑排序
     * @param nodeCount 节点数
     * @param edges 边列表(from, to)
     * @return 排序结果(空表示有环)
     */
    QVector<int> kahnSort(int nodeCount, const QVector<QPair<int, int>>& edges);

    /**
     * @brief DFS拓扑排序
     * @param nodeCount 节点数
     * @param edges 边列表(from, to)
     * @return 排序结果(空表示有环)
     */
    QVector<int> dfsSort(int nodeCount, const QVector<QPair<int, int>>& edges);

    /** @brief 检测是否存在环 */
    bool hasCycle(int nodeCount, const QVector<QPair<int, int>>& edges) const;

    /** @brief 获取检测到的环路径 */
    QVector<int> detectedCycle() const;

    /**
     * @brief 并行度分析(计算每层可并行执行的节点)
     * @param nodeCount 节点数
     * @param edges 边列表
     * @return 分层结果(每层的节点列表)
     */
    QVector<QVector<int>> parallelLevels(int nodeCount,
                                         const QVector<QPair<int, int>>& edges);

    /** @brief 计算入度表 */
    QVector<int> inDegrees(int nodeCount,
                           const QVector<QPair<int, int>>& edges) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 排序完成 @param nodeCount 节点数 @param hasCycle 是否有环 */
    void sortCompleted(int nodeCount, bool hasCycle);

private:
    /** @brief 构建邻接表 */
    QVector<QVector<int>> buildAdjList(int nodeCount,
                                       const QVector<QPair<int, int>>& edges) const;

    /** @brief DFS递归辅助 */
    bool dfsVisit(int node, const QVector<QVector<int>>& adj,
                  QVector<int>& color, QVector<int>& order,
                  QVector<int>& cycle);

    QVector<int> m_detectedCycle;

    Stats m_stats;
    double m_timeSum = 0.0;
};
