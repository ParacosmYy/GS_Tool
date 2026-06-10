/**
 * @file EdgeColoring9.h
 * @brief 边着色(Tashma迭代增广与Vizing定理的Class I/II图分类) — Edge Coloring with Tashma's Iterative Augmentation and Vizing's Theorem for Class I/II Graph Classification
 *
 * 功能: 实现边着色(edge coloring)，采用Tashma迭代增广(Tashma's iterative augmentation)
 *       与Vizing定理(Vizing's theorem)实现Class I/II图分类(Class I/II graph classification)。
 *
 * 协作: VertexColoring8(顶点着色) / GraphIsomorphism6(图同构) / Matching7(匹配)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 边着色(Tashma迭代增广与Vizing定理)
 */
class EdgeColoring9 : public QObject {
    Q_OBJECT

public:
    /** @brief Edge representation */
    struct Edge {
        int u = -1;
        int v = -1;
        int color = -1;
    };

    /** @brief Coloring result */
    struct ColoringResult {
        QVector<Edge> edges;
        int numColors = 0;
        int maxDegree = 0;
        bool isClass1 = false;    // True if chi' = Delta (Class I)
        bool isValid = false;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numVertices = 0;
        int numEdges = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit EdgeColoring9(QObject *parent = nullptr);
    ~EdgeColoring9() override;

    /** @brief Color edges of a graph given by edge list */
    ColoringResult color(const QVector<QPair<int, int>>& edges, int numVertices);

    /** @brief Validate edge coloring (no two adjacent edges share color) */
    bool validateColoring(const QVector<Edge>& coloredEdges) const;

    /** @brief Classify graph as Class I or Class II */
    bool isClassI(int maxDegree, int numColors) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void coloringDone(int edges, int colors, bool class1, double timeMs);

private:
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build adjacency structure from edge list */
    void buildAdjacency(const QVector<QPair<int, int>>& edges, int n,
                        QVector<QVector<int>>& adjEdges,
                        QVector<QVector<int>>& adjVertices) const;

    /** @brief Find maximum degree */
    int maxDegreeOf(const QVector<QVector<int>>& adjVertices) const;

    /** @brief Tashma iterative augmentation: try to recolor a Kempe chain */
    bool augmentKempe(int edgeIdx, QVector<Edge>& edges,
                      QVector<QVector<int>>& colorAt,
                      int numColors, int u, int v);

    /** @brief Find missing color at a vertex */
    int missingColor(int vertex, const QVector<QVector<int>>& colorAt,
                     int numColors) const;

    /** @brief Greedy edge coloring */
    void greedyColor(QVector<Edge>& edges, int n, int numColors);
};
