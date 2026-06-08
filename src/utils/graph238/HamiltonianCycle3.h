/**
 * @file HamiltonianCycle3.h
 * @brief 哈密顿回路(约束传播+强制边检测与路径合并剪枝) — Hamiltonian Cycle with Constraint Propagation and Pruning via Forced-Edge Detection and Path Merging
 *
 * 功能: 实现哈密顿回路搜索，采用约束传播减少搜索空间，
 *       通过强制边检测和路径合并进行剪枝优化。
 *
 * 协作: EulerPath5(欧拉路径) / TopologicalSort7(拓扑排序) / MST9(最小生成树)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 哈密顿回路(约束传播+强制边剪枝)
 */
class HamiltonianCycle3 : public QObject {
    Q_OBJECT

public:
    /** @brief Search result */
    struct CycleResult {
        QVector<int> path;
        bool found = false;
        int totalCost = 0;
        int nodesExplored = 0;
        int pruneCount = 0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numVertices = 0;
        int numEdges = 0;
        int totalNodesExplored = 0;
        int totalPrunes = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit HamiltonianCycle3(QObject *parent = nullptr);
    ~HamiltonianCycle3() override;

    /** @brief Set adjacency matrix for graph (n x n, -1 = no edge) */
    void setGraph(const QVector<QVector<int>>& adjacencyMatrix);

    /** @brief Set weighted adjacency matrix (0 = no edge, >0 = weight) */
    void setWeightedGraph(const QVector<QVector<int>>& weights);

    /** @brief Find Hamiltonian cycle starting from vertex 0 */
    CycleResult findCycle();

    /** @brief Find minimum cost Hamiltonian cycle (TSP) */
    CycleResult findMinCycle();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void searchCompleted(bool found, int pathLength, double timeMs);

private:
    int m_n = 0;
    QVector<QVector<int>> m_adj;  // Adjacency matrix (0 = no edge, 1 = edge)
    QVector<QVector<int>> m_weights; // Weight matrix

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Detect forced edges: vertices with degree 2 */
    QVector<QPair<int, int>> detectForcedEdges() const;

    /** @brief Check if adding edge (u,v) maintains degree constraint */
    bool degreeValid(int u, int v, const QVector<int>& degree) const;

    /** @brief Constraint propagation: reduce domain of possible edges */
    bool propagateConstraints(QVector<int>& degree, QVector<bool>& inPath);

    /** @brief Merge forced paths into chains */
    QVector<QVector<int>> mergePaths(const QVector<QPair<int, int>>& forcedEdges) const;

    /** @brief DFS with constraint propagation */
    bool dfsSearch(int current, int depth, QVector<int>& path,
                   QVector<bool>& visited, QVector<int>& degree,
                   CycleResult& result);

    /** @brief DFS for minimum cost cycle */
    void dfsMinCycle(int current, int depth, int cost, QVector<int>& path,
                     QVector<bool>& visited, CycleResult& best);

    /** @brief Check if graph is still potentially Hamiltonian (connectivity) */
    bool feasibilityCheck(const QVector<bool>& visited) const;
};
