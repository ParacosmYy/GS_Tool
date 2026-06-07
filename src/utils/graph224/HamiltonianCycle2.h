/**
 * @file HamiltonianCycle2.h
 * @brief 哈密顿回路搜索(回溯+度序列剪枝+旋转闭包启发式) — Hamiltonian Cycle Finder with Backtracking and Degree-Sequence Pruning with Rotational Closure Heuristic
 *
 * 功能: 实现哈密顿回路搜索，支持回溯搜索、
 *       度序列剪枝和旋转闭包启发式。
 *
 * 协作: EulerPath3(欧拉路径) / TopologicalSort4(拓扑排序) / StronglyConnected5(强连通分量)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 哈密顿回路搜索(回溯+度序列剪枝+旋转闭包启发式)
 */
class HamiltonianCycle2 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numVertices = 0;
        int numEdges = 0;
        int backtracks = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit HamiltonianCycle2(QObject *parent = nullptr);
    ~HamiltonianCycle2() override;

    /** @brief Build adjacency list from edge list */
    void setGraph(int numVertices, const QVector<QPair<int, int>>& edges);

    /** @brief Find one Hamiltonian cycle using backtracking + pruning */
    QVector<int> findCycle();

    /** @brief Find all Hamiltonian cycles */
    QVector<QVector<int>> findAllCycles();

    /** @brief Check if graph has a Hamiltonian cycle */
    bool hasHamiltonianCycle();

    /** @brief Find Hamiltonian path (not necessarily closed) */
    QVector<int> findPath();

    /** @brief Get degree sequence of current graph */
    QVector<int> degreeSequence() const;

    /** @brief Quick necessary condition check (Ore's theorem) */
    bool passesOreCondition() const;

    /** @brief Quick necessary condition check (Dirac's theorem) */
    bool passesDiracCondition() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void cycleFound(const QVector<int>& cycle, double timeMs);
    void searchCompleted(int found, int backtracks, double timeMs);

private:
    int m_n = 0;
    QVector<QVector<int>> m_adj;     // Adjacency list
    QVector<int> m_degree;           // Degree of each vertex

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Recursive backtracking search */
    bool backtrack(QVector<int>& path, QVector<bool>& visited,
                   int pos, bool findCycle);

    /** @brief Check if adding vertex v at position pos is safe */
    bool isSafe(int v, const QVector<int>& path,
                int pos, bool needCycle) const;

    /** @brief Degree-sequence pruning: check if unvisited vertices are viable */
    bool degreePrune(const QVector<bool>& visited) const;

    /** @brief Rotational closure heuristic to restart from partial path */
    QVector<int> rotationalHeuristic(const QVector<int>& partial) const;

    /** @brief Sort adjacency lists by degree for better pruning */
    void sortByDegree();
};
