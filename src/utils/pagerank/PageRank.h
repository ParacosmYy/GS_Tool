/**
 * @file PageRank.h
 * @brief PageRank算法 — 图节点重要性排序
 *
 * 功能: 实现迭代PageRank算法，支持阻尼系数配置、
 *       收敛检测、Top-K排序，统计迭代次数/收敛值/耗时。
 */
#ifndef PAGERANK_H
#define PAGERANK_H

#include <QObject>
#include <QMap>
#include <QVector>

class PageRank : public QObject {
    Q_OBJECT
public:
    /** 统计 */
    struct Stats {
        quint64 totalComputations = 0;
        quint64 totalIterations = 0;
        double  avgProcessingTimeMs = 0.0;
    };

    explicit PageRank(QObject* parent = nullptr);

    /** @brief 添加边 @param from 源节点 @param to 目标节点 */
    void addEdge(int from, int to);

    /** @brief 批量添加边 @param edges (from,to)列表 */
    void addEdges(const QVector<QPair<int, int>>& edges);

    /** @brief 计算PageRank @param damping 阻尼系数[0,1] @param maxIterations 最大迭代 @param tolerance 收敛阈值 @return node→rank */
    QMap<int, double> compute(double damping = 0.85,
                               int maxIterations = 100,
                               double tolerance = 1e-6);

    /** @brief Top-K节点 @param ranks 排名结果 @param k 数量 @return (node, rank)列表 */
    QVector<QPair<int, double>> topK(const QMap<int, double>& ranks,
                                      int k) const;

    /** @brief 清空图 */
    void clear();

    int nodeCount() const { return m_outLinks.size(); }
    int edgeCount() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void computationCompleted(int iterations, double convergence);

private:
    QMap<int, QVector<int>> m_outLinks;
    QMap<int, int> m_outDegree;
    Stats m_stats;
    double m_timeSum;
};

#endif // PAGERANK_H
