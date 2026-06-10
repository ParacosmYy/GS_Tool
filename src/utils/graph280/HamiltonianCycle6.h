/**
 * @file HamiltonianCycle6.h
 * @brief 哈密顿回路(回溯度序列剪枝与前向检查约束路径扩展) — Hamiltonian Cycle with Backtracking and Degree-Sequence Pruning Plus Forward-Checking for Constrained Path Extension
 *
 * 功能: 实现哈密顿回路(Hamiltonian cycle)，采用回溯(backtracking)和度序列剪枝(degree-sequence
 *       pruning)加前向检查(forward-checking)实现约束路径扩展(constrained path extension)。
 *
 * 协作: TSPSolver8(TSP求解) / EulerPath5(欧拉路径) / GraphColoring6(图着色)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 哈密顿回路(回溯度序列剪枝与前向检查约束路径扩展)
 */
class HamiltonianCycle6 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numVertices = 0;
        int numEdges = 0;
        bool cycleFound = false;
        int backtrackCount = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit HamiltonianCycle6(QObject *parent = nullptr);
    ~HamiltonianCycle6() override;

    /** @brief Set graph as adjacency matrix */
    void setGraph(const QVector<QVector<int>>& adjacencyMatrix);

    /** @brief Find one Hamiltonian cycle, return vertex order (empty if none) */
    QVector<int> findCycle();

    /** @brief Find all Hamiltonian cycles */
    QVector<QVector<int>> findAllCycles();

    /** @brief Check if graph has a Hamiltonian cycle */
    bool hasHamiltonianCycle();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void cycleFound(int numVertices, int backtracks, double timeMs);

private:
    int m_n = 0;
    QVector<QVector<int>> m_adj;    // Adjacency matrix
    QVector<int> m_path;            // Current path being built
    QVector<bool> m_visited;        // Visited flags
    int m_backtrackCount = 0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Backtracking search with degree pruning and forward-checking */
    bool backtrack(int pos);

    /** @brief Degree-sequence pruning: check remaining candidates */
    bool degreePruning(int currentVertex) const;

    /** @brief Forward-checking: ensure unvisited vertices remain reachable */
    bool forwardCheck(int pos) const;

    /** @brief Get degree of vertex */
    int degree(int v) const;
};
