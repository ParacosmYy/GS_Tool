/**
 * @file FlowNetwork2.h
 * @brief 最大流网络(Dinic算法) — 层次图+阻塞流
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>

/**
 * @brief 最大流网络引擎(Dinic算法)
 * 支持多次查询不同源汇、残量网络分析
 */
class FlowNetwork2 : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalMaxFlows = 0;           ///< 累计最大流计算次数
        int totalAugmentPaths = 0;       ///< 累计增广路径数
        int totalBfsLayers = 0;          ///< 累计BFS分层次数
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief 边信息 */
    struct EdgeInfo {
        int from = 0, to = 0;      ///< 端点
        double capacity = 0.0;     ///< 容量
        double flow = 0.0;         ///< 当前流量
    };

    explicit FlowNetwork2(int nodeCount, QObject* parent = nullptr);

    /** @brief 添加边 @param from 起点 @param to 终点 @param capacity 容量 @param bidirectional 是否双向 */
    void addEdge(int from, int to, double capacity, bool bidirectional = false);

    /** @brief 计算最大流 @param source 源 @param sink 汇 @return 最大流值 */
    double maxFlow(int source, int sink);

    /** @brief 获取最小割(源侧节点集) @param source 源 @return 源侧节点 */
    QVector<int> minCut(int source) const;

    /** @brief 获取所有边的流信息 */
    QList<EdgeInfo> edgeFlows() const;

    /** @brief 重置所有流量 */
    void resetFlows();

    /** @brief 获取节点数 */
    int nodeCount() const { return m_n; }
    /** @brief 获取边数 */
    int edgeCount() const { return m_graph.size() / 2; }

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 最大流计算完成 @param flow 流量值 @param paths 增广路径数 */
    void maxFlowCompleted(double flow, int paths);

private:
    /** @brief 内部边表示 */
    struct Edge {
        int to, rev;            ///< 目标节点和在邻接表中的反向边索引
        double cap;             ///< 残余容量
    };

    /** @brief BFS构建层次图 @return sink是否可达 */
    bool buildLevelGraph(int source, int sink);

    /** @brief DFS找阻塞流 */
    double sendFlow(int u, int sink, double flow, QVector<int>& iter);

    int m_n;                               ///< 节点数
    QVector<QVector<Edge>> m_graph;        ///< 邻接表
    QVector<int> m_level;                  ///< 层次图
    double m_lastMaxFlow = 0.0;           ///< 最近一次最大流值
    int m_lastSource = -1;                ///< 最近的源
    int m_lastSink = -1;                  ///< 最近的汇

    Stats m_stats;
    double m_timeSum = 0.0;
};
