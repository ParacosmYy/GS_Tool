/**
 * @file EulerTour8.h
 * @brief 欧拉回路(Hierholzer线性时间回路检测与Fleury桥边回避保证欧拉路径) — Euler Tour with Hierholzer Linear-Time Circuit Finding and Fleury's Edge-Bridge Avoidance for Guaranteed Euler Path
 *
 * 功能: 实现欧拉回路(Euler tour)，采用Hierholzer线性时间回路检测(Hierholzer linear-time)
 *       和Fleury桥边回避(Fleury's edge-bridge avoidance)实现保证欧拉路径(Euler path)。
 *
 * 协作: Dijkstra15(最短路径) / Kosaraju13(强连通分量) / TopologicalSort12(拓扑排序)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 欧拉回路(Hierholzer线性时间回路检测与Fleury桥边回避保证欧拉路径)
 */
class EulerTour8 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numVertices = 0;
        int numEdges = 0;
        int pathLength = 0;
        bool hasEulerCircuit = false;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Edge representation */
    struct Edge {
        int to = -1;
        int id = -1;       // Unique edge ID for tracking
        bool used = false;
    };

    explicit EulerTour8(QObject *parent = nullptr);
    ~EulerTour8() override;

    /** @brief Initialize graph with n vertices */
    void initGraph(int numVertices);

    /** @brief Add undirected edge */
    void addEdge(int u, int v);

    /** @brief Find Euler circuit using Hierholzer's algorithm */
    QVector<int> findEulerCircuit();

    /** @brief Find Euler path using Hierholzer with odd-degree start */
    QVector<int> findEulerPath();

    /** @brief Check if graph has Euler circuit (all degrees even, connected) */
    bool hasEulerCircuit() const;

    /** @brief Check if graph has Euler path (0 or 2 odd-degree vertices) */
    bool hasEulerPath() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void tourComputed(int pathLength, bool isCircuit, double timeMs);

private:
    int m_n = 0;
    int m_edgeCount = 0;

    QVector<QVector<Edge>> m_adj;      // Adjacency list
    QVector<int> m_degree;
    QVector<bool> m_edgeUsed;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Hierholzer DFS to find circuit starting from vertex v */
    void hierholzerDFS(int v, QVector<int>& path);

    /** @brief Check if edge (u,v) is a bridge using temporary removal */
    bool isBridge(int u, int v) const;

    /** @brief Count reachable vertices via DFS */
    int countReachable(int start, int excludeU, int excludeV) const;
};
