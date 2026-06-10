/**
 * @file HamiltonianCycle7.h
 * @brief 哈密顿回路(Ore度条件验证与Dirac定理的充分条件检查) — Hamiltonian Cycle with Ore's Degree Condition Verification and Dirac's Theorem for Sufficient Condition Checking
 *
 * 功能: 实现哈密顿回路(Hamiltonian cycle)，采用Ore度条件验证(Ore's degree condition verification)
 *       与Dirac定理(Dirac's theorem)实现充分条件检查(sufficient condition checking)。
 *
 * 协作: EulerianTrail5(欧拉路径) / TopologicalSort8(拓扑排序) / StronglyConnected6(强连通分量)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 哈密顿回路(Ore度条件验证与Dirac定理)
 */
class HamiltonianCycle7 : public QObject {
    Q_OBJECT

public:
    /** @brief Search result */
    struct CycleResult {
        QVector<int> cycle;         // Vertex indices forming the cycle
        bool hasCycle = false;
        bool oreCondition = false;  // Ore's condition satisfied
        bool diracCondition = false;// Dirac's condition satisfied
        int numBacktracks = 0;
        double searchTimeMs = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numVertices = 0;
        int numEdges = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit HamiltonianCycle7(QObject *parent = nullptr);
    ~HamiltonianCycle7() override;

    /** @brief Find Hamiltonian cycle using backtracking with pruning */
    CycleResult findCycle(const QVector<QVector<int>>& adjacency);

    /** @brief Check if graph satisfies Ore's condition (sufficient) */
    bool checkOreCondition(const QVector<QVector<int>>& adjacency) const;

    /** @brief Check if graph satisfies Dirac's condition (sufficient) */
    bool checkDiracCondition(const QVector<QVector<int>>& adjacency) const;

    /** @brief Compute degree of each vertex */
    QVector<int> computeDegrees(const QVector<QVector<int>>& adjacency) const;

    /** @brief Check if graph is complete enough for Hamiltonian path existence */
    bool isHamiltonianLikely(const QVector<QVector<int>>& adjacency) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void searchDone(int vertices, bool found, double timeMs);
    void conditionChecked(const QString& condition, bool satisfied);

private:
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Recursive backtracking search */
    bool backtrack(const QVector<QVector<int>>& adj, QVector<int>& path,
                   QVector<bool>& visited, int pos, int& backtracks);

    /** @brief Check if vertex v can be added at position pos */
    bool isSafe(int v, const QVector<QVector<int>>& adj,
                const QVector<int>& path, int pos) const;
};
