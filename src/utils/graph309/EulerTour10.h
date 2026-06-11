/**
 * @file EulerTour10.h
 * @brief 欧拉回路(Hierholzer电路遍历与Fleury避桥保证混合图中欧拉路径/回路) — Euler Tour with Hierholzer Circuit Traversal and Fleury Bridge Avoidance for Guaranteed Euler Path/Circuit in Mixed Graphs
 *
 * 功能: 实现欧拉回路(Euler tour)，采用Hierholzer电路遍历(Hierholzer circuit traversal)
 *       与Fleury避桥(Fleury bridge avoidance)保证混合图中欧拉路径/回路(Euler path/circuit in mixed graphs)。
 *
 * 协作: HamiltonianCycle9(哈密顿回路) / TopologicalSort8(拓扑排序) / StronglyConnected7(强连通分量)
 */
#pragma once

#include <QObject>
#include <QVector>

class EulerTour10 : public QObject {
    Q_OBJECT

public:
    /** @brief Edge representation */
    struct Edge {
        int to = -1;
        int id = -1;        // Unique edge ID for multi-graph support
        bool used = false;
    };

    /** @brief Euler tour result */
    struct TourResult {
        QVector<int> vertexPath;
        QVector<int> edgePath;
        bool isCircuit = false;
        bool isValid = false;
        int totalEdges = 0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalSearches = 0;
        int numVertices = 0;
        int numEdges = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit EulerTour10(QObject *parent = nullptr);
    ~EulerTour10() override;

    /** @brief Add undirected edge */
    void addEdge(int u, int v);

    /** @brief Reset graph */
    void clear();

    /** @brief Find Euler tour using Hierholzer's algorithm */
    TourResult findEulerTour();

    /** @brief Check if graph has Euler circuit */
    bool hasEulerCircuit() const;

    /** @brief Check if graph has Euler path */
    bool hasEulerPath() const;

    int vertexCount() const { return m_numVertices; }
    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void tourFound(int vertices, int edges, bool isCircuit, double timeMs);

private:
    int m_numVertices = 0;
    int m_edgeCounter = 0;
    QVector<QVector<Edge>> m_adj;
    QVector<int> m_degree;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Ensure vertex exists in adjacency list */
    void ensureVertex(int v);

    /** @brief Hierholzer's algorithm core */
    void hierholzer(int start, QVector<int>& circuit, QVector<int>& edges);

    /** @brief Count unused edges from a vertex */
    int unusedEdgeCount(int v) const;
};
