/**
 * @file VertexCover6.h
 * @brief 顶点覆盖(LP松弛半整数性+皇冠分解核化) — Vertex Cover with LP Relaxation Half-Integrality and Kernelization via Crown Decomposition
 *
 * 功能: 实现顶点覆盖近似算法，利用LP松弛的半整数性质，
 *       通过皇冠分解进行核化降低问题规模。
 *
 * 协作: MaxCut5(最大割) / GraphColoring7(图着色) / Matching9(匹配)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 顶点覆盖(LP松弛+皇冠分解核化)
 */
class VertexCover6 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numVertices = 0;
        int numEdges = 0;
        int coverSize = 0;
        int kernelSize = 0;
        double lpOptimal = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit VertexCover6(QObject *parent = nullptr);
    ~VertexCover6() override;

    /** @brief Build graph from edge list (vertices: 0..n-1) */
    void buildGraph(int n, const QVector<QPair<int, int>>& edges);

    /** @brief Add edge to graph */
    void addEdge(int u, int v);

    /** @brief Solve vertex cover using LP relaxation + crown kernelization */
    QVector<int> solve();

    /** @brief Solve LP relaxation, return fractional values */
    QVector<double> solveLPRelaxation() const;

    /** @brief Perform crown decomposition kernelization */
    QPair<QVector<int>, QVector<QPair<int, int>>>
    crownKernelize(const QVector<double>& lpSolution) const;

    /** @brief Check if vertex set is a valid cover */
    bool isValidCover(const QVector<int>& cover) const;

    /** @brief Get adjacency list */
    QVector<QVector<int>> adjacencyList() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveCompleted(int coverSize, int kernelSize, double timeMs);

private:
    int m_n = 0;
    QVector<QVector<int>> m_adj;      // Adjacency list
    QVector<QPair<int, int>> m_edges;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Find maximum matching for crown detection */
    QVector<QPair<int, int>> findMaxMatching(
        const QVector<int>& leftSet, const QVector<int>& rightSet) const;

    /** @brief BFS-based augmenting path for matching */
    bool augmentPath(int u, QVector<int>& matchL, QVector<int>& matchR,
                     QVector<bool>& visited) const;

    /** @brief Round LP half-integral solution to integer cover */
    QVector<int> roundLPSolution(const QVector<double>& lpSol) const;
};
