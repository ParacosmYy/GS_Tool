/**
 * @file MinimumSpanningTree2.h
 * @brief 最小生成树(Borůvka算法+并行分量收缩) — Minimum Spanning Tree via Borůvka's Algorithm with Parallel Component Contraction
 *
 * 功能: 实现Borůvka最小生成树算法，支持并行分量收缩、
 *       Union-Find连通分量管理和边权重排序。
 *
 * 协作: Dijkstra7(最短路径) / TopologicalSort3(拓扑排序) / StronglyConnected4(强连通分量)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 最小生成树(Borůvka算法+并行分量收缩)
 */
class MinimumSpanningTree2 : public QObject {
    Q_OBJECT

public:
    /** @brief Weighted edge */
    struct Edge {
        int from = 0;
        int to = 0;
        double weight = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalRuns = 0;
        int numVertices = 0;
        int numEdges = 0;
        double totalWeight = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit MinimumSpanningTree2(QObject *parent = nullptr);
    ~MinimumSpanningTree2() override;

    void setNumVertices(int n);
    void setEdges(const QVector<Edge>& edges);

    /** @brief Run Borůvka's algorithm, return MST edges */
    QVector<Edge> boruvka();

    /** @brief Run with parallel component contraction */
    QVector<Edge> boruvkaParallel();

    /** @brief Compute total weight of edge set */
    static double totalWeight(const QVector<Edge>& edges);

    /** @brief Check if edge set forms a valid spanning tree */
    bool isValidMST(const QVector<Edge>& mstEdges) const;

    /** @brief Build adjacency list from edge list */
    QVector<QVector<QPair<int, double>>> adjacencyList() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void mstCompleted(int vertices, int edges, double weight, double timeMs);

private:
    int m_n = 0;
    QVector<Edge> m_edges;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Union-Find: find with path compression */
    int ufFind(QVector<int>& parent, int x) const;

    /** @brief Union-Find: union by rank */
    bool ufUnion(QVector<int>& parent, QVector<int>& rank, int a, int b) const;

    /** @brief Contract components: remap vertices */
    QVector<Edge> contractEdges(const QVector<int>& component,
                                 int numComponents) const;
};
