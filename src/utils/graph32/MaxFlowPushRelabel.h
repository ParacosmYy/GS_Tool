/**
 * @file MaxFlowPushRelabel.h
 * @brief Push-Relabel最大流 — 最高标号选择/间隔启发式/全局重标号
 *
 * 功能: 实现Push-Relabel最大流算法，支持最高标号选择策略，
 *       间隔(gap)启发式优化，全局重标号操作，discharge操作，
 *       用于网络流分析、资源分配和调度优化。
 *
 * 协作: DataFlowMeter(流量分析) / DataBatchProcessor(批处理调度)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QMap>
#include <QPair>

/**
 * @brief Push-Relabel最大流算法 — 最高标号/间隔/全局重标号优化
 */
class MaxFlowPushRelabel : public QObject {
    Q_OBJECT

public:
    /** @brief 边信息 */
    struct Edge {
        int from = 0;              ///< 起点
        int to = 0;                ///< 终点
        double capacity = 0.0;     ///< 容量
        double flow = 0.0;         ///< 当前流量
        int rev = 0;               ///< 反向边索引
    };

    /** @brief 统计信息 */
    struct Stats {
        quint64 totalPushes = 0;            ///< 累计push操作次数
        quint64 totalRelabels = 0;          ///< 累计relabel操作次数
        quint64 totalGapRelabels = 0;       ///< 累计gap启发式触发次数
        quint64 totalGlobalRelabels = 0;    ///< 累计全局重标号次数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理时间(ms)
        double  lastMaxFlow = 0.0;          ///< 最近计算的最大流
        int     lastVertexCount = 0;        ///< 最近计算的顶点数
    };

    explicit MaxFlowPushRelabel(QObject* parent = nullptr);

    /** @brief 设置顶点数 @param n 顶点数(≥2) */
    void setVertexCount(int n);

    /** @brief 添加边 @param from 起点 @param to 终点 @param capacity 容量 */
    void addEdge(int from, int to, double capacity);

    /** @brief 清空所有边 */
    void clearEdges();

    /** @brief 计算最大流(Push-Relabel) @param source 源点 @param sink 汇点 @return 最大流值 */
    double maxFlow(int source, int sink);

    /** @brief 最高标号选择策略 @param source 源点 @param sink 汇点 @return 最大流值 */
    double maxFlowHighestLabel(int source, int sink);

    /** @brief 带全局重标号的最大流 @param source 源点 @param sink 汇点 @param relabelInterval 全局重标号间隔 @return 最大流值 */
    double maxFlowWithGlobalRelabel(int source, int sink, int relabelInterval = 50);

    /** @brief 获取最小割 @param source 源点 @return (S集合, T集合) */
    QPair<QVector<int>, QVector<int>> minCut(int source) const;

    /** @brief 获取边流量 @return 所有边的信息 */
    QVector<Edge> edges() const;

    /** @brief 获取指定顶点的邻接边 @param v 顶点 @return 边列表 */
    QVector<Edge> vertexEdges(int v) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 最大流计算完成 @param flow 最大流值 @param pushes push次数 @param relabels relabel次数 */
    void maxFlowComputed(double flow, quint64 pushes, quint64 relabels);

    /** @brief 间隔启发式触发 @param gapHeight 间隔高度 */
    void gapHeuristicTriggered(int gapHeight);

private:
    void push(int v, int edgeIdx);
    void relabel(int v);
    void discharge(int v);
    void gapHeuristic(int gapHeight);
    void globalRelabel(int source, int sink);
    void initPreflow(int source);
    bool isAdmissible(int v, int edgeIdx) const;

    int m_n;                       ///< 顶点数
    int m_source;                  ///< 源点
    int m_sink;                    ///< 汇点

    QVector<QVector<Edge>> m_adj;  ///< 邻接表
    QVector<double> m_excess;      ///< 超额流量
    QVector<int> m_height;         ///< 高度标号
    QVector<int> m_current;        ///< 当前边指针
    QVector<bool> m_inQueue;       ///< 是否在活跃队列中

    Stats m_stats;
    double m_timeSum = 0.0;        ///< 处理时间累加器
};
