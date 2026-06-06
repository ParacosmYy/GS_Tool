/**
 * @file MinimumSpanningTree.h
 * @brief 最小生成树(Kruskal并查集+Prim堆优化) — Minimum Spanning Tree via Kruskal with Union-Find and Prim's Heap-based Variant
 *
 * 功能: 实现最小生成树算法，支持Kruskal(并查集路径压缩+按秩合并)、
 *       Prim(二叉堆优化)和总权值计算。
 *
 * 协作: Dijkstra6(Dijkstra) / BellmanFord7(Bellman-Ford) / Graph10(图基础)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 最小生成树(Kruskal+Prim)
 */
class MinimumSpanningTree : public QObject {
    Q_OBJECT

public:
    /** @brief Weighted edge */
    struct Edge {
        int from = 0;
        int to = 0;
        double weight = 0.0;
    };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalRuns = 0;
        int numVertices = 0;
        int numEdges = 0;
        double totalWeight = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit MinimumSpanningTree(QObject *parent = nullptr);
    ~MinimumSpanningTree() override;

    /** @brief 设置邻接表图(顶点数) */
    void setNumVertices(int n);

    /** @brief 添加边 */
    void addEdge(int from, int to, double weight);

    /** @brief Kruskal算法求MST */
    QVector<Edge> kruskal();

    /** @brief Prim算法求MST(堆优化) */
    QVector<Edge> prim();

    /** @brief 计算MST总权值 */
    double totalWeight(const QVector<Edge>& mst) const;

    /** @brief 清空图 */
    void clear();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void mstComputed(const QString& method, int numEdges, double totalWeight);

private:
    int m_numVertices = 0;
    QVector<Edge> m_edges;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Union-Find: find with path compression */
    int ufFind(QVector<int>& parent, QVector<int>& rank_, int x) const;

    /** @brief Union-Find: union by rank */
    bool ufUnion(QVector<int>& parent, QVector<int>& rank_,
                 int x, int y) const;
};
