/**
 * @file EulerTour7.h
 * @brief 欧拉回路(Fleury割边检测+边栈Hierholzer高效路径构建) — Euler Tour with Fleury's Bridge Detection and Edge-Stack Based Hierholzer for Efficient Path Construction
 *
 * 功能: 实现欧拉回路(Euler Tour/Circuit)算法，使用Fleury割边检测
 *       (Fleury's bridge detection)避免过早断开图连通性，结合边栈
 *       Hierholzer(edge-stack based Hierholzer)高效构建完整欧拉路径。
 *
 * 协作: TopologicalSort5(拓扑排序) / Dijkstra6(最短路径) / BFS3(广度搜索)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 欧拉回路(Fleury割边检测+边栈Hierholzer)
 */
class EulerTour7 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numVertices = 0;
        int numEdges = 0;
        int tourLength = 0;
        int numBridgeChecks = 0;
        int numComponents = 0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Edge representation */
    struct Edge {
        int to = -1;
        int id = -1;        // Unique edge ID for tracking
        bool used = false;
    };

    explicit EulerTour7(QObject *parent = nullptr);
    ~EulerTour7() override;

    /** @brief Build graph from edge list (undirected) */
    void buildGraph(int numVertices, const QVector<QPair<int,int>>& edges);

    /** @brief Find Eulerian tour using Hierholzer with bridge awareness */
    QVector<int> findEulerTour();

    /** @brief Check if graph has Eulerian circuit */
    bool hasEulerCircuit() const;

    /** @brief Check if graph has Eulerian path */
    bool hasEulerPath() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void tourCompleted(int length, double timeMs);

private:
    int m_n = 0;
    int m_edgeCount = 0;
    QVector<QVector<Edge>> m_adj;   // Adjacency list
    QVector<int> m_edgeUsed;        // Track used edges by ID

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Count vertices reachable from v */
    int countReachable(int v, QVector<bool>& visited) const;

    /** @brief Check if edge (u, edgeIdx) is a bridge using DFS */
    bool isBridge(int u, int edgeIdx) const;

    /** @brief Find valid starting vertex for Euler path */
    int findStartVertex() const;

    /** @brief DFS-based connected component count */
    int countComponents() const;
};
