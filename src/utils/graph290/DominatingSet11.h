/**
 * @file DominatingSet11.h
 * @brief 支配集(贪心随机自适应搜索与局部搜索改进的元启发式优化) — Dominating Set with Greedy Randomized Adaptive Search and Local Search Improvement for Metaheuristic Optimization
 *
 * 功能: 实现支配集(Dominating set)，采用贪心随机自适应搜索(greedy randomized adaptive search)
 *       与局部搜索改进(local search improvement)实现元启发式优化(metaheuristic optimization)。
 *
 * 协作: GraphColoring10(图着色) / MaxClique10(最大团) / VertexCover10(顶点覆盖)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 支配集(贪心随机自适应搜索与局部搜索改进)
 */
class DominatingSet11 : public QObject {
    Q_OBJECT

public:
    /** @brief Solution result */
    struct Solution {
        QVector<int> vertices;     // Dominating set vertices
        int size = 0;              // Number of vertices in set
        double cost = 0.0;         // Weighted cost
        bool feasible = false;     // All vertices dominated
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numVertices = 0;
        int numEdges = 0;
        int bestSize = 0;
        int numIterations = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit DominatingSet11(QObject *parent = nullptr);
    ~DominatingSet11() override;

    /** @brief Load graph from adjacency list */
    void setGraph(const QVector<QVector<int>>& adjacency);

    /** @brief Set vertex weights for weighted domination */
    void setWeights(const QVector<double>& weights);

    /** @brief Set GRASP parameters */
    void setGRASPParams(int maxIterations, double alpha);

    /** @brief Solve with greedy randomized adaptive search */
    Solution solve();

    /** @brief Solve with pure greedy (fast, no randomization) */
    Solution solveGreedy();

    /** @brief Verify if a set is a valid dominating set */
    bool verify(const QVector<int>& candidateSet) const;

    /** @brief Get adjacency list */
    QVector<QVector<int>> graph() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solutionFound(int setSize, double cost, int iteration, double timeMs);

private:
    QVector<QVector<int>> m_adj;    // Adjacency list
    QVector<double> m_weights;      // Vertex weights
    int m_maxIter = 100;
    double m_alpha = 0.3;           // RCL parameter [0,1]

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build greedy randomized solution */
    Solution constructGreedyRandomized();

    /** @brief Improve solution via local search */
    Solution localSearch(const Solution& sol);

    /** @brief Check if vertex v is dominated by given set */
    bool isDominated(int v, const QSet<int>& domSet) const;

    /** @brief Count undominated vertices */
    int countUndominated(const QSet<int>& domSet) const;

    /** @brief Compute coverage gain of adding vertex v */
    int coverageGain(int v, const QSet<int>& domSet) const;

    /** @brief Compute weighted cost of a solution */
    double computeCost(const QVector<int>& vertices) const;
};
