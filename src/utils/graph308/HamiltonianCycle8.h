/**
 * @file HamiltonianCycle8.h
 * @brief 哈密顿回路(度序列剪枝与旋转邻接序回溯搜索实现提前终止的HC检测) — Hamiltonian Cycle with Pruning by Degree Sequence and Rotated Adjacency Ordering for Backtracking Search with Early Termination
 *
 * 功能: 实现哈密顿回路检测(Hamiltonian cycle detection)，采用度序列剪枝(pruning by degree sequence)
 *       与旋转邻接序(rotated adjacency ordering)实现提前终止的回溯搜索(backtracking search with early termination)。
 *
 * 协作: EulerTour7(欧拉回路) / TopologicalSort6(拓扑排序) / StronglyConnected5(强连通分量)
 */
#pragma once

#include <QObject>
#include <QVector>

class HamiltonianCycle8 : public QObject {
    Q_OBJECT

public:
    /** @brief Search result */
    struct CycleResult {
        QVector<int> cycle;         // Vertex indices forming the cycle
        bool found = false;
        int nodesExplored = 0;
        int pruningCuts = 0;
        double timeMs = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalSearches = 0;
        int numVertices = 0;
        int numEdges = 0;
        int cyclesFound = 0;
        double avgNodesExplored = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit HamiltonianCycle8(QObject *parent = nullptr);
    ~HamiltonianCycle8() override;

    /** @brief Set adjacency matrix (n x n, symmetric for undirected) */
    void setGraph(const QVector<QVector<int>>& adjacency);

    /** @brief Set adjacency matrix with edge weights */
    void setWeightedGraph(const QVector<QVector<double>>& weights, double threshold = 0.0);

    /** @brief Find a Hamiltonian cycle starting from vertex 0 */
    CycleResult findCycle();

    /** @brief Find all Hamiltonian cycles (limited by maxCycles) */
    QVector<CycleResult> findAllCycles(int maxCycles = 10);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void searchDone(bool found, int pathLength, double timeMs);

private:
    int m_n = 0;
    QVector<QVector<int>> m_adj;    // Adjacency matrix (0/1)
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_nodesSum = 0.0;

    /** @brief Degree of each vertex */
    QVector<int> m_degree;

    /** @brief Rotated adjacency ordering for each vertex */
    QVector<QVector<int>> m_adjOrder;

    /** @brief Backtracking search with pruning */
    bool backtrack(QVector<int>& path, QVector<bool>& visited,
                    int pos, CycleResult& result);

    /** @brief Check necessary conditions for Hamiltonian cycle existence */
    bool necessaryConditions() const;

    /** @brief Build rotated adjacency ordering based on degree */
    void buildAdjacencyOrder();

    /** @brief Check if adding vertex v at position pos is valid */
    bool isSafe(int v, const QVector<int>& path, int pos) const;
};
