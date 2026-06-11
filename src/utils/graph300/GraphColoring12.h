/**
 * @file GraphColoring12.h
 * @brief 图着色(进化变异与Tabu搜索元启发式着色数最小化) — Graph Coloring with Evolutionary Mutation and Tabu Search for Metaheuristic Chromatic Number Minimization
 *
 * 功能: 实现图着色(Graph coloring)，采用进化变异(evolutionary mutation)
 *       与Tabu搜索(Tabu search)实现元启发式着色数最小化(metaheuristic chromatic number minimization)。
 *
 * 协作: GraphIsomorphism7(图同构) / MaxFlow7(最大流) / MST9(最小生成树)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 图着色(进化变异与Tabu搜索元启发式着色数最小化)
 */
class GraphColoring12 : public QObject {
    Q_OBJECT

public:
    /** @brief Coloring result */
    struct ColoringResult {
        QVector<int> colors;           // Color assignment per vertex
        int chromaticNum = 0;          // Number of colors used
        int conflicts = 0;
        int iterations = 0;
        bool valid = false;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numVertices = 0;
        int numEdges = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit GraphColoring12(QObject *parent = nullptr);
    ~GraphColoring12() override;

    void setAdjacencyList(const QVector<QVector<int>>& adj);
    void setAdjacencyMatrix(const QVector<QVector<int>>& matrix);

    void setMaxIterations(int iters);
    void setPopulationSize(int size);
    void setMutationRate(double rate);
    void setTabuTenure(int tenure);

    /** @brief Color graph with evolutionary Tabu search */
    ColoringResult color();

    /** @brief Try to color with exactly k colors */
    ColoringResult colorWithK(int k);

    /** @brief Find minimum chromatic number via binary search */
    ColoringResult findMinColoring();

    /** @brief Verify coloring validity */
    bool verifyColoring(const QVector<int>& colors) const;

    int vertexCount() const { return m_n; }
    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void coloringDone(int colors, int conflicts, double timeMs);

private:
    int m_n = 0;
    int m_maxIter = 500;
    int m_popSize = 20;
    double m_mutationRate = 0.15;
    int m_tabuTenure = 10;
    Stats m_stats;
    double m_timeSum = 0.0;

    QVector<QVector<int>> m_adj;           // Adjacency list
    QVector<QVector<bool>> m_adjMatrix;    // Adjacency matrix

    /** @brief Count color conflicts */
    int countConflicts(const QVector<int>& colors) const;

    /** @brief Initialize random coloring with k colors */
    QVector<int> randomColoring(int k, unsigned int& seed) const;

    /** @brief Tabu search local improvement */
    QVector<int> tabuImprove(const QVector<int>& colors, int k, int maxIter);

    /** @brief Evolutionary crossover of two colorings */
    QVector<int> crossover(const QVector<int>& p1, const QVector<int>& p2,
                           int k, unsigned int& seed) const;

    /** @brief Mutation: randomly recolor a vertex */
    void mutate(QVector<int>& colors, int k, unsigned int& seed) const;

    /** @brief Greedy DSATUR initial coloring */
    QVector<int> dsaturColoring() const;

    /** @brief Get saturation degree for DSATUR */
    int satDegree(int v, const QVector<int>& colors,
                  const QVector<bool>& used) const;
};
