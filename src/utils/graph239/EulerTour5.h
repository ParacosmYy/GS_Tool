/**
 * @file EulerTour5.h
 * @brief 欧拉巡回(Fleury桥检测算法+安全边选择博物馆路线规划) — Euler Tour with Fleury's Bridge-Checking Algorithm and Safe Edge Selection for Museum Tour Planning
 *
 * 功能: 实现欧拉巡回/欧拉路径查找，采用Fleury桥检测算法选择安全边，
 *       支持博物馆参观路线规划等实际应用场景。
 *
 * 协作: Dijkstra12(最短路径) / DFSIterator9(深度优先遍历) / BFSIterator8(广度优先)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QMap>

/**
 * @brief 欧拉巡回(Fleury桥检测+博物馆路线规划)
 */
class EulerTour5 : public QObject {
    Q_OBJECT

public:
    /** @brief Edge representation */
    struct Edge {
        int from = 0;
        int to = 0;
        double weight = 1.0;
        int id = 0;
        bool used = false;
    };

    /** @brief Tour result */
    struct TourResult {
        QVector<int> vertexPath;
        QVector<int> edgePath;
        bool isEulerian = false;
        bool isClosedTour = false;
        double totalWeight = 0.0;
        int numBridgesChecked = 0;
        QString failureReason;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numVertices = 0;
        int numEdges = 0;
        int numBridgesChecked = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit EulerTour5(QObject *parent = nullptr);
    ~EulerTour5() override;

    /** @brief Add undirected edge */
    void addEdge(int u, int v, double weight = 1.0);

    /** @brief Clear graph */
    void clear();

    /** @brief Find Euler tour (circuit or path) using Fleury's algorithm */
    TourResult findEulerTour();

    /** @brief Check if graph has Eulerian circuit */
    bool hasEulerianCircuit() const;

    /** @brief Check if graph has Eulerian path */
    bool hasEulerianPath() const;

    /** @brief Plan museum tour visiting all corridors */
    TourResult planMuseumTour(const QMap<int, QString>& roomNames);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void tourFound(int vertices, int edges, bool closed, double timeMs);

private:
    int m_nextEdgeId = 0;
    QMap<int, QVector<Edge>> m_adjacency;
    QVector<Edge> m_edges;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Count reachable vertices via DFS */
    int countReachable(int start, const QMap<int, QVector<Edge>>& adj) const;

    /** @brief Check if removing edge disconnects the graph (bridge check) */
    bool isBridge(int u, int edgeId) const;

    /** @brief Remove edge from adjacency lists */
    void removeEdge(int edgeId);

    /** @brief Get vertex degrees */
    QMap<int, int> computeDegrees() const;

    /** @brief Find valid start vertex for Euler path/circuit */
    int findStartVertex() const;

    /** @brief DFS for reachability counting */
    void dfsReachable(int v, const QMap<int, QVector<Edge>>& adj,
                       QMap<int, bool>& visited) const;
};
