/**
 * @file MinCut.h
 * @brief 最小割 — Stoer-Wagner全局最小割算法
 *
 * 功能: 计算无向带权图的全局最小割，将顶点集划分为两个不相交子集，
 *       使割边总权重最小。使用Stoer-Wagner算法，复杂度O(V^3)。
 *
 * 协作: MaxFlow(最大流) / GraphCluster(图聚类)
 */
#ifndef MINCUT_H
#define MINCUT_H

#include <QObject>
#include <QVector>

/**
 * @brief 全局最小割计算器
 *
 * Stoer-Wagner算法通过V-1轮最大权重扩张来寻找全局最小割。
 * 每轮选择一个顶点加入集合A，最后加入的两个顶点进行收缩。
 */
class MinCut : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalComputed = 0;         ///< 累计计算次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit MinCut(QObject* parent = nullptr);

    /**
     * @brief 计算全局最小割权重
     * @param adjacency 邻接矩阵(adjacency[i][j]为边权重)
     * @return 最小割的边总权重
     */
    double compute(const QVector<QVector<double>>& adjacency);

    /**
     * @brief 获取最小割的划分结果
     * @return 第一个分区的顶点索引列表
     */
    QVector<int> cutPartition() const;

    /** @brief 获取统计信息 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief 计算完成信号 @param cutWeight 最小割权重 */
    void computationCompleted(double cutWeight);

private:
    Stats m_stats;                      ///< 统计信息
    double m_timeSumMs = 0.0;           ///< 累计耗时(ms)
    QVector<int> m_partition;           ///< 最小割划分结果
};

#endif // MINCUT_H
