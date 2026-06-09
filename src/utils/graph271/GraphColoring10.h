/**
 * @file GraphColoring10.h
 * @brief 图着色(递归最大优先+独立集提取增量着色) — Graph Coloring with Recursive Largest First and Independent Set Extraction for Incremental Coloring
 *
 * 功能: 实现图着色(Graph Coloring)，采用递归最大优先(Recursive Largest
 *       First, RLF)算法，配合独立集提取(independent set extraction)
 *       实现增量着色(incremental coloring)。
 *
 * 协作: TopologicalSort6(拓扑排序) / MST7(最小生成树) / BFS3(广度优先搜索)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 图着色(递归最大优先+独立集提取增量着色)
 */
class GraphColoring10 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numVertices = 0;
        int numEdges = 0;
        int numColors = 0;
        int numIndependentSets = 0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Coloring result */
    struct ColoringResult {
        QVector<int> colors;
        int numColors = 0;
        bool isValid = false;
    };

    /** @brief Independent set info */
    struct IndependentSet {
        QVector<int> vertices;
        int color = 0;
    };

    explicit GraphColoring10(QObject *parent = nullptr);
    ~GraphColoring10() override;

    /** @brief Set adjacency list (graph representation) */
    void setGraph(const QVector<QVector<int>>& adjacency);

    /** @brief Add a vertex with edges */
    void addVertex(const QVector<int>& neighbors);

    /** @brief Remove a vertex */
    void removeVertex(int v);

    /** @brief Add an edge between u and v */
    void addEdge(int u, int v);

    /** @brief Run RLF coloring algorithm */
    ColoringResult colorRLF();

    /** @brief Verify coloring validity */
    bool verifyColoring(const QVector<int>& colors) const;

    /** @brief Extract independent sets from current coloring */
    QVector<IndependentSet> extractIndependentSets() const;

    /** @brief Incrementally recolor after graph modification */
    ColoringResult recolorIncremental(const QVector<int>& changedVertices);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void coloringCompleted(int numColors, int numVertices, double timeMs);

private:
    QVector<QVector<int>> m_adj;       // Adjacency list
    int m_numVertices = 0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Find vertex with most uncolored neighbors */
    int findMaxDegree(const QVector<bool>& colored) const;

    /** @brief Find uncolored vertex with most neighbors in set */
    int findMaxAdjInSet(const QVector<bool>& colored,
                        const QVector<bool>& inSet) const;

    /** @brief Extract one independent set via RLF */
    IndependentSet extractOneIS(QVector<bool>& colored, int colorIdx);

    /** @brief Greedy sequential coloring for comparison */
    QVector<int> greedyColoring() const;
};
