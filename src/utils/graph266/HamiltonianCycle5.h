/**
 * @file HamiltonianCycle5.h
 * @brief 哈密顿回路(Held-Karp动态规划+位掩码状态表示) — Hamiltonian Cycle with Held-Karp Dynamic Programming and Bitmask State Representation for Small Graph Exact Solution
 *
 * 功能: 实现哈密顿回路检测(Hamiltonian Cycle)，使用Held-Karp动态规划
 *       (dynamic programming)配合位掩码状态表示(bitmask state)对小规模
 *       图求精确解，时间复杂度O(2^N * N^2)。
 *
 * 协作: EulerPath3(欧拉路径) / ShortestPath4(最短路径) / MST6(最小生成树)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 哈密顿回路(Held-Karp动态规划+位掩码)
 */
class HamiltonianCycle5 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numVertices = 0;
        int numEdges = 0;
        int numDPStates = 0;
        int numHamiltonianCycles = 0;
        double minCycleWeight = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit HamiltonianCycle5(QObject *parent = nullptr);
    ~HamiltonianCycle5() override;

    /** @brief Set adjacency matrix (N x N, 0 = no edge) */
    void setGraph(const QVector<QVector<double>>& adjMatrix);

    /** @brief Find minimum weight Hamiltonian cycle, return vertex path */
    QVector<int> findMinCycle();

    /** @brief Check if Hamiltonian cycle exists */
    bool hasCycle() const;

    /** @brief Get cycle weight (valid after findMinCycle) */
    double cycleWeight() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void cycleFound(int numVertices, double weight, double timeMs);

private:
    int m_n = 0;
    QVector<QVector<double>> m_adj; // Adjacency matrix
    QVector<int> m_bestPath;
    double m_bestWeight = 0.0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Held-Karp DP: dp[mask][v] = min cost to reach v through mask */
    QVector<QVector<double>> m_dp;

    /** @brief Parent table for path reconstruction */
    QVector<QVector<int>> m_parent;

    /** @brief Run Held-Karp DP */
    void heldKarp();

    /** @brief Reconstruct path from parent table */
    QVector<int> reconstructPath() const;

    /** @brief Count set bits in mask */
    static int popcount(int mask);
};
