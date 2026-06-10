/**
 * @file EulerTour9.h
 * @brief 欧拉游走(Hierholzer顶点栈回路查找与邻接表边删除的高效欧拉路径/回路) — Euler Tour with Hierholzer Vertex-stack Circuit Finding and Adjacency List Edge Deletion for Efficient Eulerian Path/Circuit
 *
 * 功能: 实现欧拉游走(Euler tour)，采用Hierholzer顶点栈回路查找(Hierholzer vertex-stack circuit finding)
 *       与邻接表边删除(adjacency list edge deletion)实现高效欧拉路径/回路(Eulerian path/circuit)。
 *
 * 协作: DFS8(深度优先搜索) / BFS7(广度优先搜索) / Dijkstra10(最短路径)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 欧拉游走(Hierholzer顶点栈回路查找与邻接表边删除)
 */
class EulerTour9 : public QObject {
    Q_OBJECT

public:
    /** @brief Edge in the graph */
    struct Edge {
        int to = -1;
        int edgeId = -1;   // Unique edge identifier for deletion
        bool used = false;
    };

    /** @brief Tour result */
    struct TourResult {
        QVector<int> vertexPath;     // Sequence of vertices
        QVector<int> edgePath;       // Sequence of edge IDs
        bool isEulerian = false;
        bool isCircuit = false;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numVertices = 0;
        int numEdges = 0;
        double avgProcessingTimeMs = 0.0;
        int tourLength = 0;
    };

    explicit EulerTour9(QObject *parent = nullptr);
    ~EulerTour9() override;

    /** @brief Set number of vertices */
    void setNumVertices(int n);

    /** @brief Add undirected edge between u and v */
    void addEdge(int u, int v);

    /** @brief Build graph from edge list (pairs of vertices) */
    void buildGraph(const QVector<QPair<int, int>>& edges);

    /** @brief Find Eulerian circuit (all vertices with even degree) */
    TourResult findEulerianCircuit();

    /** @brief Find Eulerian path (exactly 2 vertices with odd degree) */
    TourResult findEulerianPath();

    /** @brief Check if graph has Eulerian circuit */
    bool hasEulerianCircuit() const;

    /** @brief Check if graph has Eulerian path */
    bool hasEulerianPath() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void tourFound(int vertices, int edges, bool isCircuit, double timeMs);
    void vertexVisited(int vertex, int stackSize);

private:
    int m_n = 0;
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_edgeCounter = 0;

    /** @brief Adjacency list for each vertex */
    QVector<QVector<Edge>> m_adj;

    /** @brief Degree count per vertex */
    QVector<int> m_degree;

    /** @brief Hierholzer algorithm with vertex stack */
    QVector<int> hierholzer(int startVertex);

    /** @brief Find starting vertex for Eulerian path (odd degree vertex) */
    int findPathStart() const;

    /** @brief Delete used edge from adjacency list */
    void deleteEdge(int u, int edgeId);
};
