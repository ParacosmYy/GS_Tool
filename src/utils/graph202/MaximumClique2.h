/**
 * @file MaximumClique2.h
 * @brief 最大团(Tomita着色上界剪枝+分支限界) — Maximum Clique via Branch-and-Bound with Tomita Color-Based Upper Bound Pruning
 *
 * 功能: 实现最大团搜索算法，支持Tomita着色上界剪枝、
 *       递归分支限界、贪心着色排序和团大小渐进枚举。
 *
 * 协作: GraphColoring4(图着色) / MaximumIndependentSet3(最大独立集) / BronKerbosch2(极大团枚举)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 最大团搜索器(Tomita着色上界+分支限界)
 */
class MaximumClique2 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalSearches = 0;
        int numVertices = 0;
        int maxCliqueSize = 0;
        qint64 nodesExplored = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit MaximumClique2(QObject *parent = nullptr);
    ~MaximumClique2() override;

    /** @brief 设置邻接矩阵(false=无边, true=有边) */
    void setAdjacencyMatrix(const QVector<QVector<bool>>& adj);

    /** @brief 设置邻接表 */
    void setAdjacencyList(const QVector<QVector<int>>& adjList);

    /** @brief 搜索最大团，返回顶点索引 */
    QVector<int> findMaximumClique();

    /** @brief 贪心着色(返回每个顶点的颜色编号) */
    QVector<int> greedyColoring(const QVector<int>& vertices) const;

    /** @brief 获取团大小 */
    int maxCliqueSize() const { return m_bestClique.size(); }

    const QVector<int>& bestClique() const { return m_bestClique; }
    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void searchCompleted(int cliqueSize, qint64 nodesExplored, double timeMs);

private:
    QVector<QVector<bool>> m_adjMatrix;
    QVector<QVector<int>> m_adjList;
    int m_n = 0;

    QVector<int> m_bestClique;
    qint64 m_nodesExplored = 0;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Recursive branch-and-bound with Tomita pruning */
    void expand(QVector<int>& current, QVector<int>& candidates);

    /** @brief Tomita-style coloring with sorting (returns color bounds) */
    QVector<int> colorSort(QVector<int>& candidates) const;

    /** @brief Check if vertex v is adjacent to all vertices in the set */
    bool isCliqueMember(int v, const QVector<int>& clique) const;

    /** @brief Build adjacency list from matrix */
    void buildAdjList();
};
