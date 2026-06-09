/**
 * @file EulerTour6.h
 * @brief 欧拉回路(Hierholzer算法+Fleury边选择启发式+保证路径扩展) — Euler Tour with Hierholzer Algorithm and Fleury's Edge-Selection Heuristic for Guaranteed Path Extension
 *
 * 功能: 实现欧拉回路(Euler tour)查找，采用Hierholzer算法(Hierholzer algorithm)构建
 *       欧拉回路，结合Fleury边选择启发式(Fleury's edge-selection heuristic)优先选择
 *       非桥边避免过早断开连通分量，保证路径扩展(guaranteed path extension)。
 *
 * 协作: Dijkstra12(最短路径) / StronglyConnected6(强连通分量) / TopologicalSort5(拓扑排序)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 欧拉回路(Hierholzer算法+Fleury边选择启发式+保证路径扩展)
 */
class EulerTour6 : public QObject {
    Q_OBJECT

public:
    /** @brief Edge representation */
    struct Edge {
        int to = -1;
        int id = -1;        // unique edge id for multi-edges
        bool used = false;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numVertices = 0;
        int numEdges = 0;
        int tourLength = 0;
        bool hasEulerTour = false;
        int numBridgesAvoided = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit EulerTour6(QObject *parent = nullptr);
    ~EulerTour6() override;

    /** @brief Build directed graph with n vertices */
    void buildDirected(int n);

    /** @brief Build undirected graph with n vertices */
    void buildUndirected(int n);

    /** @brief Add edge (u -> v) or undirected edge */
    void addEdge(int u, int v);

    /** @brief Find Euler tour (circuit or path) */
    QVector<int> findTour();

    /** @brief Check if graph has Euler circuit */
    bool hasEulerCircuit() const;

    /** @brief Check if graph has Euler path */
    bool hasEulerPath() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void tourFound(int length, bool isCircuit, double timeMs);
    void edgeTraversed(int u, int v, int edgeId);

private:
    int m_n = 0;
    bool m_directed = false;
    int m_edgeCounter = 0;

    QVector<QVector<Edge>> m_adj;   // adjacency list
    QVector<int> m_edgeCount;        // degree per vertex

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Check if removing edge (u,idx) disconnects the graph (bridge check) */
    bool isBridge(int u, int edgeIdx);

    /** @brief Count reachable vertices from v via DFS */
    int countReachable(int v, QVector<bool>& visited) const;

    /** @brief Fleury's heuristic: select best edge from vertex u */
    int fleurySelectEdge(int u);

    /** @brief Hierholzer's algorithm for directed graph */
    QVector<int> hierholzerDirected();

    /** @brief Hierholzer's algorithm with Fleury for undirected */
    QVector<int> hierholzerUndirected();
};
