/**
 * @file DominatingSet3.h
 * @brief 最小支配集(贪心近似+未来覆盖度打破平局) — Minimum Dominating Set via Greedy Approximation with Tie-Breaking by Future Coverage
 *
 * 功能: 实现最小支配集贪心近似算法，支持覆盖率贪心选择、
 *       未来覆盖度打破平局和顶点权重优化。
 *
 * 协作: VertexCover4(顶点覆盖) / MaxIndependentSet3(最大独立集) / GraphColor5(图着色)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 最小支配集求解器
 */
class DominatingSet3 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalSolves = 0;           ///< 累计求解次数
        int lastSetSize = 0;               ///< 最近支配集大小
        double avgProcessingTimeMs = 0.0;  ///< 平均耗时(ms)
    };

    explicit DominatingSet3(QObject *parent = nullptr);
    ~DominatingSet3() override;

    /**
     * @brief 求解最小支配集(邻接表)
     * @param adjList 邻接表(adjList[v] = 邻居列表)
     * @return 支配集中的顶点索引
     */
    QVector<int> solve(const QVector<QVector<int>>& adjList);

    /**
     * @brief 验证支配集是否有效
     * @param adjList 邻接表
     * @param domSet 支配集
     * @return true=有效
     */
    bool validate(const QVector<QVector<int>>& adjList,
                  const QVector<int>& domSet) const;

    /** @brief 获取每个顶点的覆盖状态 */
    QVector<bool> coverageMap() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveCompleted(int setSize);

private:
    /** @brief 计算顶点当前覆盖增益 */
    int coverageGain(int vertex,
                     const QVector<QVector<int>>& adjList,
                     const QVector<bool>& dominated) const;

    /** @brief 计算未来覆盖度(2-hop邻居未覆盖数) */
    int futureCoverage(int vertex,
                       const QVector<QVector<int>>& adjList,
                       const QVector<bool>& dominated) const;

    QVector<bool> m_dominated;

    Stats m_stats;
    double m_timeSum = 0.0;
};
