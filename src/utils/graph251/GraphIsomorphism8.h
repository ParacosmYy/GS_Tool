/**
 * @file GraphIsomorphism8.h
 * @brief 图同构(顶点分类迭代细化+色彩传递至稳定划分) — Graph Isomorphism via Iterative Vertex Classification Refinement with Color-Passing Until Stable Partition
 *
 * 功能: 实现图同构检测(Graph isomorphism)，采用顶点分类迭代细化(iterative vertex classification
 *       refinement)和色彩传递(color-passing)算法，持续细化顶点颜色签名直到达到稳定划分(stable
 *       partition)，通过比较两个图的稳定色分区判定同构性。
 *
 * 协作: GraphColoring7(图着色) / VF2Matcher6(VF2匹配) / SubgraphSearch5(子图搜索)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 图同构(顶点分类迭代细化+色彩传递至稳定划分)
 */
class GraphIsomorphism8 : public QObject {
    Q_OBJECT

public:
    /** @brief Isomorphism result */
    struct IsoResult {
        bool isomorphic = false;
        QVector<int> mapping;    // mapping[i] = j means vertex i in G1 maps to j in G2
        int iterations = 0;
        int numColorClasses = 0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numVertices1 = 0;
        int numVertices2 = 0;
        int numEdges1 = 0;
        int numEdges2 = 0;
        int totalIterations = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit GraphIsomorphism8(QObject *parent = nullptr);
    ~GraphIsomorphism8() override;

    /** @brief Set first graph as adjacency list */
    void setGraph1(const QVector<QVector<int>>& adj);

    /** @brief Set second graph as adjacency list */
    void setGraph2(const QVector<QVector<int>>& adj);

    /** @brief Check if two graphs are isomorphic */
    IsoResult check() const;

    /** @brief Set max refinement iterations */
    void setMaxIterations(int iter);

    /** @brief Get canonical coloring of a graph */
    QVector<int> canonicalColoring(const QVector<QVector<int>>& adj) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void refinementIteration(int iter, int colorClasses);
    void checkCompleted(bool iso, int iters, double timeMs);

private:
    int m_maxIter = 200;
    QVector<QVector<int>> m_adj1;
    QVector<QVector<int>> m_adj2;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Initial coloring by degree */
    QVector<int> initialColoring(const QVector<QVector<int>>& adj) const;

    /** @brief One round of color refinement (color-passing) */
    QVector<int> refineColors(const QVector<QVector<int>>& adj,
                              const QVector<int>& colors) const;

    /** @brief Check if color partition is stable */
    bool isStable(const QVector<int>& prev, const QVector<int>& curr) const;

    /** @brief Count distinct color classes */
    int countColorClasses(const QVector<int>& colors) const;

    /** @brief Build color multiset signature for a vertex */
    QVector<int> neighborSignature(const QVector<QVector<int>>& adj,
                                   const QVector<int>& colors, int v) const;

    /** @brief Relabel colors to consecutive integers */
    QVector<int> relabelColors(const QVector<int>& colors) const;

    /** @brief Verify candidate mapping by edge consistency */
    bool verifyMapping(const QVector<QVector<int>>& g1,
                       const QVector<QVector<int>>& g2,
                       const QVector<int>& map) const;

    /** @brief Backtracking search for exact mapping */
    bool findMapping(const QVector<QVector<int>>& g1,
                     const QVector<QVector<int>>& g2,
                     const QVector<int>& c1, const QVector<int>& c2,
                     QVector<int>& map, QVector<bool>& used, int depth) const;
};
