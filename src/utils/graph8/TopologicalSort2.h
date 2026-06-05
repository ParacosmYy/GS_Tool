/**
 * @file TopologicalSort2.h
 * @brief 拓扑排序(Kahn算法)
 */

#pragma once

#include <QObject>
#include <QVector>

/**
 * @class TopologicalSort2
 * @brief Kahn拓扑排序 — BFS式拓扑排序，可检测环
 *
 * 基于入度的拓扑排序，适用于任务调度、编译依赖、课程安排等DAG场景。
 * 如果排序结果不包含所有节点，说明图中存在环。
 */
class TopologicalSort2 : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalSorts = 0;         /**< 总排序次数 */
        int cyclesDetected = 0;     /**< 检测到环的次数 */
        double avgProcessingTimeMs = 0.0; /**< 平均处理时间(ms) */
    };

    /** @brief 构造函数 */
    explicit TopologicalSort2(QObject* parent = nullptr);

    /**
     * @brief 设置有向图
     * @param adj 邻接表: adj[i] = {i指向的节点列表}
     * @param numNodes 节点数
     */
    void setGraph(const QVector<QVector<int>>& adj, int numNodes);

    /**
     * @brief 执行拓扑排序
     * @return 排序结果(如果有环，结果不包含全部节点)
     */
    QVector<int> sort();

    /**
     * @brief 检查图中是否有环
     * @return 是否有环
     */
    bool hasCycle() const;

    /**
     * @brief 添加边
     * @param from 起点
     * @param to 终点
     */
    void addEdge(int from, int to);

    /** @brief 获取统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 排序完成信号 */
    void sortCompleted(int nodeCount, bool hasCycle);

private:
    QVector<QVector<int>> m_adj;
    int m_numNodes;
    bool m_lastHadCycle;

    Stats m_stats;
    double m_timeSum;
};
