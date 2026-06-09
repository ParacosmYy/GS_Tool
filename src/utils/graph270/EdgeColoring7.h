/**
 * @file EdgeColoring7.h
 * @brief 边着色(Vizing定理分类+贪心近优边着色分配) — Edge Coloring with Vizing's Theorem Classification and Greedy Edge-Color Assignment for Near-Optimal Coloring
 *
 * 功能: 实现图边着色(Edge coloring)，基于Vizing定理分类(Vizing's
 *       theorem classification)确定色数范围，贪心边着色分配(greedy
 *       edge-color assignment)实现近优着色(near-optimal coloring)。
 *
 * 协作: GraphColoring4(顶点着色) / BipartiteMatching3(二分匹配) / VertexCover5(顶点覆盖)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 边着色(Vizing定理分类+贪心近优着色)
 */
class EdgeColoring7 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numVertices = 0;
        int numEdges = 0;
        int maxDegree = 0;
        int colorsUsed = 0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief An edge in the graph */
    struct Edge {
        int u = -1;
        int v = -1;
        int color = -1;
    };

    /** @brief Vizing classification result */
    enum class GraphClass { Class1, Class2, Unknown };

    explicit EdgeColoring7(QObject *parent = nullptr);
    ~EdgeColoring7() override;

    /** @brief Set graph adjacency (edge list, pairs of vertices) */
    void setGraph(int numVertices, const QVector<QPair<int,int>>& edges);

    /** @brief Run greedy edge coloring */
    QVector<Edge> color();

    /** @brief Classify graph per Vizing's theorem */
    GraphClass classifyGraph() const;

    /** @brief Get maximum vertex degree */
    int maxDegree() const;

    /** @brief Get minimum colors needed (chromatic index lower bound) */
    int lowerBound() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void coloringCompleted(int colorsUsed, int edges, double timeMs);

private:
    int m_numVertices = 0;
    int m_maxDeg = 0;
    QVector<Edge> m_edges;

    // Adjacency for conflict checking: color -> set of vertices
    QVector<QVector<int>> m_adjColors;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute maximum vertex degree */
    void computeMaxDegree();

    /** @brief Check if color is available for edge at vertex */
    bool isColorAvailable(int edgeIdx, int color) const;

    /** @brief Find smallest available color for an edge */
    int findSmallestColor(int edgeIdx) const;

    /** @brief Check if graph is regular (all degrees equal) */
    bool isRegular() const;

    /** @brief Check if graph has odd cycle */
    bool hasOddCycle() const;

    /** @brief Count degree of a vertex */
    int vertexDegree(int v) const;
};
