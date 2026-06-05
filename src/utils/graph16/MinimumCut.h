/**
 * @file MinimumCut.h
 * @brief Karger随机化最小割算法 — 边收缩求全局最小割
 *
 * 功能:
 *   - Karger随机收缩: 多次随机选择边收缩直到只剩两个超级顶点
 *   - 支持带权/无权无向图
 *   - 可配置迭代次数提高成功率
 *   - 返回割大小及对应的两个顶点分区
 */

#pragma once

#include <QObject>
#include <QVector>
#include <QPair>
#include <QMap>
#include <QSet>

/**
 * @class MinimumCut
 * @brief Karger随机化最小割算法引擎
 *
 * 通过多次随机边收缩逼近无向图的全局最小割。
 * 单次成功概率 O(1/n^2)，迭代 O(n^2 log n) 次可高概率保证最优解。
 */
class MinimumCut : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalRuns = 0;               /**< 总运行次数 */
        int totalContractions = 0;       /**< 总边收缩次数 */
        int totalEdges = 0;              /**< 总处理边数 */
        double avgProcessingTimeMs = 0.0; /**< 平均处理时间(ms) */
    };

    /** @brief 割结果 */
    struct CutResult {
        int cutSize = 0;                 /**< 割大小(割边数/权值和) */
        QVector<QSet<int>> partitions;   /**< 两个分区的顶点集合 */
        bool valid = false;              /**< 结果是否有效 */
    };

    /** @brief 边: (顶点u, 顶点v, 权重) */
    using WeightedEdge = QTriple<int, int, double>;

    /** @brief 构造函数 */
    explicit MinimumCut(QObject* parent = nullptr);

    /**
     * @brief Karger最小割(无权图)
     * @param numVertices 顶点数
     * @param edges 边列表(u, v)
     * @param iterations 迭代次数(0=自动n^2*log(n))
     * @return 最小割结果
     */
    CutResult findMinCut(int numVertices,
                          const QVector<QPair<int, int>>& edges,
                          int iterations = 0) const;

    /**
     * @brief Karger最小割(带权图)
     * @param numVertices 顶点数
     * @param edges 加权边列表
     * @param iterations 迭代次数
     * @return 最小割结果(割大小=权重和)
     */
    CutResult findMinCutWeighted(int numVertices,
                                   const QVector<WeightedEdge>& edges,
                                   int iterations = 0) const;

    /**
     * @brief 单次Karger收缩
     * @param numVertices 顶点数
     * @param edges 边列表
     * @return 本次割结果
     */
    CutResult singleContraction(int numVertices,
                                  const QVector<QPair<int, int>>& edges) const;

    /** @brief 获取统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 割计算完成 */
    void cutFound(int cutSize, int iterations);

private:
    /** @brief 并查集: 查找根 */
    int findRoot(QVector<int>& parent, int x) const;

    /** @brief 并查集: 合并 */
    void unionSets(QVector<int>& parent, QVector<int>& rank,
                   int a, int b) const;

    /** @brief 从边列表提取分区 */
    QVector<QSet<int>> extractPartitions(
        const QVector<int>& parent, int numVertices) const;

    mutable Stats m_stats;           /**< 统计信息 */
    mutable double m_timeSum = 0.0;  /**< 累计时间 */
};
