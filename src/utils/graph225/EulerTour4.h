/**
 * @file EulerTour4.h
 * @brief 欧拉路径(Hierholzer遍历+多重图边着色路径分解) — Euler Tour with Hierholzer Traversal and Edge-Coloring for Multi-Graph Trail Decomposition
 *
 * 功能: 实现欧拉路径/回路，支持Hierholzer遍历算法、
 *       多重图边着色和路径分解。
 *
 * 协作: TopologicalSort4(拓扑排序) / StronglyConnected5(强连通分量) / MinSpanningTree6(最小生成树)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 欧拉路径(Hierholzer遍历+多重图边着色路径分解)
 */
class EulerTour4 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numVertices = 0;
        int numEdges = 0;
        int numTrails = 0;
        bool hasEulerCircuit = false;
        bool hasEulerPath = false;
        double avgProcessingTimeMs = 0.0;
    };

    explicit EulerTour4(QObject *parent = nullptr);
    ~EulerTour4() override;

    /** @brief Set directed or undirected graph */
    void setDirected(bool directed);

    /** @brief Add edge (u -> v) with optional multiplicity */
    void addEdge(int u, int v, int count = 1);

    /** @brief Build graph from adjacency list */
    void buildGraph(int numVertices,
                    const QVector<QVector<QPair<int, int>>>& adj);

    /** @brief Check if Euler circuit exists */
    bool hasEulerianCircuit() const;

    /** @brief Check if Euler path exists */
    bool hasEulerianPath() const;

    /** @brief Find Euler circuit using Hierholzer's algorithm */
    QVector<int> findEulerCircuit();

    /** @brief Find Euler path */
    QVector<int> findEulerPath();

    /** @brief Decompose graph into minimum number of trails */
    QVector<QVector<int>> decomposeTrails();

    /** @brief Edge-color the multigraph for trail decomposition */
    QVector<QVector<QPair<int, int>>> edgeColoring() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void tourFound(const QVector<int>& path, double timeMs);

private:
    bool m_directed = false;
    int m_numVertices = 0;

    // Adjacency: for each vertex, list of (neighbor, edgeId)
    QVector<QVector<QPair<int, int>>> m_adj;

    // Edge list: (from, to)
    QVector<QPair<int, int>> m_edges;

    // Used edge tracking
    QVector<bool> m_edgeUsed;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Hierholzer traversal from start vertex */
    QVector<int> hierholzer(int start);

    /** @brief Count vertex degrees */
    void computeDegrees(QVector<int>& inDeg, QVector<int>& outDeg) const;

    /** @brief Find Euler start vertex */
    int findStartVertex() const;
};
