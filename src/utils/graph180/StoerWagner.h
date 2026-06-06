/**
 * @file StoerWagner.h
 * @brief Stoer-Wagner全局最小割(最大邻接序) — Stoer-Wagner Global Minimum Cut via Maximum Adjacency Ordering
 *
 * 功能: 实现Stoer-Wagner全局最小割算法，通过最大邻接(MA)序反复收缩顶点，
 *       求无向加权图的全局最小割。时间复杂度O(n(m + n log n))。
 *
 * 协作: EdmondsKarp(最大流) / PrimMST(最小生成树) / KruskalMST(最小生成树)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief Stoer-Wagner全局最小割求解器
 */
class StoerWagner : public QObject {
    Q_OBJECT

public:
    /** @brief 割结果 */
    struct CutResult {
        double weight;                   ///< 割权重
        QVector<int> partitionA;         ///< 分区A顶点
        QVector<int> partitionB;         ///< 分区B顶点
    };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalRuns = 0;           ///< 累计运行次数
        double avgProcessingTimeMs = 0.0;///< 平均处理耗时(ms)
        int lastVertexCount = 0;         ///< 最近一次顶点数
        int lastEdgeCount = 0;           ///< 最近一次边数
    };

    explicit StoerWagner(QObject* parent = nullptr);
    ~StoerWagner() override;

    /**
     * @brief 设置邻接矩阵(对称加权无向图)
     * @param matrix 邻接矩阵，matrix[i][j]为边权
     */
    void setAdjacencyMatrix(const QVector<QVector<double>>& matrix);

    /**
     * @brief 从边列表构建图
     * @param n 顶点数
     * @param edges 边列表(顶点u, 顶点v, 权重w)
     */
    void setEdgeList(int n, const QVector<QPair<QPair<int, int>, double>>& edges);

    /**
     * @brief 计算全局最小割
     * @return 最小割结果
     */
    CutResult computeMinCut();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 最小割计算完成 @param weight 割权重 */
    void minCutComputed(double weight);

private:
    /** @brief 单次MA序收缩阶段 */
    QPair<double, QPair<int, int>> minimumCutPhase(const QVector<bool>& merged,
                                                    const QVector<QVector<double>>& adj) const;

    /** @brief 最大邻接序(MA ordering) */
    QVector<int> maOrdering(const QVector<bool>& merged,
                            const QVector<QVector<double>>& adj) const;

    int m_n = 0;
    QVector<QVector<double>> m_adjMatrix;

    Stats m_stats;
    double m_timeSum = 0.0;
};
