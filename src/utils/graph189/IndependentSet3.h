/**
 * @file IndependentSet3.h
 * @brief 最大独立集(分支限界+度排序顶点序) — Maximum Independent Set via Branch-and-Bound with Degree-Based Vertex Ordering
 *
 * 功能: 实现无向图最大独立集求解，支持基于顶点度的排序策略、
 *       分支限界剪枝和上界估计。
 *
 * 协作: VertexCover(顶点覆盖) / GraphColoring(图着色) / CliqueFinder(团查找)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 最大独立集求解器(分支限界)
 */
class IndependentSet3 : public QObject {
    Q_OBJECT

public:
    /** @brief 顶点排序策略 */
    enum Ordering { NaturalOrder = 0, DegreeAscending = 1, DegreeDescending = 2 };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalSolves = 0;           ///< 累计求解次数
        int bestSize = 0;                  ///< 最大独立集大小
        int nodesExplored = 0;             ///< 最近搜索节点数
        int pruningCount = 0;              ///< 最近剪枝次数
        double avgProcessingTimeMs = 0.0;  ///< 平均耗时(ms)
    };

    explicit IndependentSet3(QObject *parent = nullptr);
    ~IndependentSet3() override;

    void setOrdering(Ordering order);
    void setTimeLimitMs(int ms);

    /**
     * @brief 从邻接矩阵求解最大独立集
     * @param adjacency 邻接矩阵(adj[i][j]=1表示有边)
     * @return 最大独立集中的顶点索引
     */
    QVector<int> solve(const QVector<QVector<int>>& adjacency);

    /**
     * @brief 从边列表求解
     * @param edges 边列表({u, v})
     * @param numVertices 顶点数
     * @return 最大独立集中的顶点索引
     */
    QVector<int> solveFromEdges(const QVector<QPair<int, int>>& edges,
                                 int numVertices);

    /** @brief 获取独立集大小(不返回具体集合) */
    int maxSize(const QVector<QVector<int>>& adjacency);

    /** @brief 验证集合是否为独立集 */
    bool validateIndependentSet(const QVector<QVector<int>>& adjacency,
                                const QVector<int>& set) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveCompleted(int setSize, int nodesExplored);
    void progressUpdate(int depth, int currentBest);

private:
    /** @brief 分支限界搜索 */
    void branchAndBound(const QVector<QVector<int>>& adj,
                        const QVector<int>& ordering);

    /** @brief 递归搜索 */
    void search(const QVector<QVector<int>>& adj,
                const QVector<int>& ordering,
                QVector<int>& current,
                QVector<bool>& excluded,
                int idx);

    /** @brief 计算上界(贪心着色数) */
    int upperBound(const QVector<QVector<int>>& adj,
                   const QVector<int>& ordering,
                   const QVector<bool>& excluded,
                   int startIdx) const;

    /** @brief 按度排序顶点 */
    QVector<int> orderByDegree(const QVector<QVector<int>>& adj) const;

    Ordering m_ordering = DegreeDescending;
    int m_timeLimitMs = 30000;

    QVector<int> m_bestSet;         ///< 当前最优独立集
    int m_bestSize = 0;             ///< 当前最优大小
    int m_nodesExplored = 0;        ///< 搜索节点数
    int m_pruningCount = 0;         ///< 剪枝次数

    Stats m_stats;
    double m_timeSum = 0.0;
};
