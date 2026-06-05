/**
 * @file MaxFlow.h
 * @brief 最大流算法(Edmonds-Karp) — 基于BFS的Ford-Fulkerson实现
 *
 * 功能: 实现Edmonds-Karp最大流算法，在容量网络上计算
 *       从源点到汇点的最大流量。统计计算次数/节点数/平均耗时。
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QQueue>

/**
 * @class MaxFlow
 * @brief Edmonds-Karp最大流计算器
 *
 * 利用BFS寻找增广路径，保证O(VE^2)时间复杂度。
 * 返回最大流量及各边的实际流量分布。
 */
class MaxFlow : public QObject
{
    Q_OBJECT

public:
    /** @brief 运行统计信息 */
    struct Stats {
        quint64 totalComputed = 0;    ///< 总计算次数
        quint64 totalNodes = 0;       ///< 累计处理节点数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    /**
     * @brief 构造函数
     * @param parent QObject父对象
     */
    explicit MaxFlow(QObject* parent = nullptr);

    /**
     * @brief 计算最大流
     * @param capacity 邻接矩阵表示的容量图
     * @param source 源点索引
     * @param sink 汇点索引
     * @return 最大流量值
     */
    double compute(const QVector<QVector<double>>& capacity, int source, int sink);

    /**
     * @brief 获取最近一次计算的流量分布
     * @return 邻接矩阵表示的流量图
     */
    QVector<QVector<double>> getFlow() const;

    /** @brief 获取统计信息 */
    Stats stats() const { return m_stats; }

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief 计算完成信号 @param maxFlow 最大流量值 */
    void computationCompleted(double maxFlow);

private:
    /**
     * @brief BFS寻找增广路径
     * @param residual 残余容量图
     * @param source 源点
     * @param sink 汇点
     * @param parent 记录路径前驱(输出参数)
     * @return 是否找到增广路径
     */
    bool bfs(const QVector<QVector<double>>& residual,
             int source, int sink,
             QVector<int>& parent) const;

    mutable Stats m_stats;                        ///< 统计信息
    double m_timeSum = 0.0;                       ///< 累计耗时
    QVector<QVector<double>> m_lastFlow;          ///< 最近计算的流量矩阵
};
