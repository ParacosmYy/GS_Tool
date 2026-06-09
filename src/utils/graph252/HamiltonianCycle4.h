/**
 * @file HamiltonianCycle4.h
 * @brief 哈密顿回路(回溯搜索+度序列剪枝+强制边传播) — Hamiltonian Cycle with Backtracking Search and Degree-Sequence Pruning with Forced-Edge Propagation
 *
 * 功能: 实现哈密顿回路搜索算法(Hamiltonian cycle search)，采用回溯搜索(backtracking search)
 *       遍历顶点排列空间，结合度序列剪枝(degree-sequence pruning)提前排除无效分支，通过
 *       强制边传播(forced-edge propagation)确定必选边并简化搜索图。
 *
 * 协作: EulerPath5(欧拉路径) / GraphColoring8(图着色) / TopologicalSort6(拓扑排序)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 哈密顿回路(回溯搜索+度序列剪枝+强制边传播)
 */
class HamiltonianCycle4 : public QObject {
    Q_OBJECT

public:
    /** @brief Search result */
    struct SearchResult {
        QVector<int> path;
        bool found = false;
        int nodesExplored = 0;
        int prunedBranches = 0;
        double timeMs = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numVertices = 0;
        int numEdges = 0;
        int totalNodesExplored = 0;
        int totalPrunedBranches = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit HamiltonianCycle4(QObject *parent = nullptr);
    ~HamiltonianCycle4() override;

    /** @brief Load adjacency matrix for graph with n vertices */
    void setGraph(int n, const QVector<QVector<int>>& adjacency);

    /** @brief Add forced edge that must appear in cycle */
    void addForcedEdge(int u, int v);

    /** @brief Clear all forced edges */
    void clearForcedEdges();

    /** @brief Set max search nodes before aborting (0 = unlimited) */
    void setMaxNodes(int limit);

    /** @brief Search for a Hamiltonian cycle starting from vertex 0 */
    SearchResult search();

    /** @brief Search for all Hamiltonian cycles up to limit */
    QVector<QVector<int>> searchAll(int maxCycles = 10);

    /** @brief Check if current graph can potentially have a Hamiltonian cycle */
    bool isFeasible() const;

    /** @brief Get current degree sequence */
    QVector<int> degreeSequence() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void searchCompleted(bool found, int nodesExplored, double timeMs);
    void cycleFound(const QVector<int>& path);

private:
    int m_n = 0;
    QVector<QVector<int>> m_adj;        // adjacency list
    QVector<QVector<int>> m_adjMatrix;  // adjacency matrix
    QVector<QPair<int,int>> m_forced;   // forced edges
    int m_maxNodes = 0;

    // Search state
    QVector<int> m_path;
    QVector<bool> m_inPath;
    int m_nodesExplored = 0;
    int m_prunedBranches = 0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Degree-sequence pruning: check Ore/Erdos conditions */
    bool passesDegreePruning() const;

    /** @brief Forced-edge propagation: detect 2-degree forced paths */
    void propagateForcedEdges(QVector<QVector<int>>& workAdj) const;

    /** @brief Recursive backtracking with pruning */
    bool backtrack(int pos, QVector<QVector<int>>& workAdj);

    /** @brief Check if vertex v is safe to add at position pos */
    bool isSafe(int v, int pos, const QVector<QVector<int>>& workAdj) const;

    /** @brief Count vertices with degree <= 1 (pruning signal) */
    int countLowDegree(const QVector<QVector<int>>& adj) const;
};
