/**
 * @file HamiltonianCycle.h
 * @brief 哈密顿回路检测(回溯+Warnsdorff启发式+度剪枝) — Hamiltonian Cycle Detection via Backtracking with Warnsdorff's Heuristic and Degree-Based Pruning
 *
 * 功能: 实现哈密顿回路检测算法，支持回溯搜索、Warnsdorff启发式排序
 *       加速剪枝、基于度数的提前终止和邻接表图表示。
 *
 * 协作: Dijkstra10(最短路径) / BellmanFord6(BF最短路) / TopologicalSort4(拓扑排序)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 哈密顿回路检测器(回溯+Warnsdorff+度剪枝)
 */
class HamiltonianCycle : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalSearches = 0;
        int numVertices = 0;
        int numEdges = 0;
        int nodesExplored = 0;
        int pruningCuts = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit HamiltonianCycle(QObject *parent = nullptr);
    ~HamiltonianCycle() override;

    /** @brief Build graph from adjacency list (vertex -> neighbors) */
    void setGraph(const QVector<QVector<int>>& adjList);

    /** @brief Add edge (undirected) */
    void addEdge(int u, int v);

    /** @brief Clear graph */
    void clearGraph();

    /** @brief Find one Hamiltonian cycle (empty if none) */
    QVector<int> findCycle();

    /** @brief Check if a Hamiltonian cycle exists */
    bool hasCycle();

    /** @brief Find all Hamiltonian cycles (up to maxCount) */
    QVector<QVector<int>> findAllCycles(int maxCount = 10);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void searchCompleted(int vertices, bool found, double timeMs);

private:
    int m_numVertices = 0;
    QVector<QVector<int>> m_adj;
    int m_maxCount = 10;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Backtracking search for Hamiltonian cycle */
    bool backtrack(QVector<int>& path, QVector<bool>& visited, int pos);

    /** @brief Backtracking with Warnsdorff ordering */
    bool backtrackWarnsdorff(QVector<int>& path, QVector<bool>& visited, int pos);

    /** @brief Get neighbors sorted by Warnsdorff heuristic (ascending degree) */
    QVector<int> warnsdorffOrder(int vertex, const QVector<bool>& visited) const;

    /** @brief Degree-based pruning: all vertices must have degree >= 2 */
    bool degreePruning() const;

    /** @brief Multi-cycle backtracking */
    void backtrackAll(QVector<int>& path, QVector<bool>& visited,
                      int pos, QVector<QVector<int>>& results);
};
