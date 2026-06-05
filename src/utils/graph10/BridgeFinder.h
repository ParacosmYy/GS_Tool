/**
 * @file BridgeFinder.h
 * @brief 无向图桥与割点检测器 — Tarjan算法
 *
 * 功能: 在无向图中寻找桥(割边)和关节点(割点)，
 *       判断图的双连通性，使用Tarjan DFS算法。
 *
 * 协作: CycleDetector(环路检测) / DataCorrelator(关联分析)
 * @author Serial Tool Team
 * @date 2026-06-05
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>
#include <QtGlobal>

/**
 * @class BridgeFinder
 * @brief 无向图桥与割点检测器
 *
 * 使用Tarjan深度优先搜索算法在O(V+E)时间内
 * 寻找无向图中的所有桥和关节点。需先调用setGraph()
 * 设置邻接表，再调用查找方法。
 */
class BridgeFinder : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息结构体 */
    struct Stats {
        quint64 totalSearches = 0;         ///< 累计搜索次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    /** @brief 构造函数 @param parent 父对象 */
    explicit BridgeFinder(QObject *parent = nullptr);

    /**
     * @brief 设置无向图邻接表
     * @param adj 邻接表，adj[i]包含节点i的所有邻居
     * @param numNodes 节点总数
     */
    void setGraph(const QVector<QVector<int>> &adj, int numNodes);

    /**
     * @brief 查找所有桥(割边)
     * @return 桥的列表，每对表示桥的两个端点(u,v)，u<v
     */
    QVector<QPair<int, int>> findBridges();

    /**
     * @brief 查找所有关节点(割点)
     * @return 关节点编号列表
     */
    QVector<int> findArticulationPoints();

    /**
     * @brief 判断图是否双连通(无关节点)
     * @return true表示双连通
     */
    bool isBiconnected();

    /** @brief 获取统计信息 @return 常量引用 */
    const Stats &stats() const { return m_stats; }

    /** @brief 重置统计计数器 */
    void resetStatistics();

signals:
    /** @brief 搜索完成信号 @param bridgeCount 桥数量 @param articulationCount 割点数量 */
    void searchCompleted(int bridgeCount, int articulationCount);

private:
    /**
     * @brief Tarjan DFS递归遍历
     * @param u 当前节点
     * @param parent 父节点(-1表示根)
     * @param timer 全局时间戳引用
     * @param disc 发现时间数组
     * @param low Low值数组
     * @param visited 访问标记数组
     * @param bridges 桥结果输出
     * @param articulations 割点结果输出
     */
    void tarjanDfs(int u, int parent, int &timer,
                   QVector<int> &disc, QVector<int> &low,
                   QVector<bool> &visited,
                   QVector<QPair<int, int>> &bridges,
                   QVector<int> &articulations);

    int m_numNodes = 0;                     ///< 节点总数
    QVector<QVector<int>> m_adj;            ///< 邻接表
    Stats m_stats;                          ///< 统计信息
    double m_timeSum = 0.0;                 ///< 处理时间累加器
};
