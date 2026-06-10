/**
 * @file GraphIsomorphism13.h
 * @brief 图同构(个体化-细化与搜索树剪枝认证非同构测试) — Graph Isomorphism with Individualization-Refinement and Search Tree Pruning for Certified Non-isomorphism Testing
 *
 * 功能: 实现图同构检测(Graph isomorphism)，采用个体化-细化(individualization-refinement)
 *       与搜索树剪枝(search tree pruning)实现认证非同构测试(certified non-isomorphism testing)。
 *
 * 协作: VF2Graph11(VF2匹配) / BronKerbosch12(团枚举) / GraphColoring11(图着色)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 图同构(个体化-细化与搜索树剪枝)
 */
class GraphIsomorphism13 : public QObject {
    Q_OBJECT

public:
    /** @brief Search tree node for individualization-refinement */
    struct SearchNode {
        QVector<int> coloring;      // Current vertex coloring
        int individualized = -1;    // Last individualized vertex
        int depth = 0;              // Depth in search tree
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numVertices = 0;
        int numEdges = 0;
        int searchNodesExplored = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit GraphIsomorphism13(QObject *parent = nullptr);
    ~GraphIsomorphism13() override;

    /** @brief Check if two adjacency matrices represent isomorphic graphs */
    bool isIsomorphic(const QVector<QVector<int>>& adj1,
                      const QVector<QVector<int>>& adj2);

    /** @brief Get a vertex mapping if isomorphic, empty otherwise */
    QVector<int> findMapping(const QVector<QVector<int>>& adj1,
                             const QVector<QVector<int>>& adj2);

    /** @brief Compute canonical label (certificate) for a graph */
    QVector<int> canonicalLabel(const QVector<QVector<int>>& adj);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void testDone(bool isomorphic, int nodesExplored, double timeMs);

private:
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Refine coloring using neighbor color multisets */
    bool refine(const QVector<QVector<int>>& adj, QVector<int>& coloring) const;

    /** @brief Get color class sizes for pruning comparison */
    QVector<int> colorClassSizes(const QVector<int>& coloring, int n) const;

    /** @brief Individualize a vertex: give it a unique new color */
    QVector<int> individualize(const QVector<int>& coloring, int vertex, int n) const;

    /** @brief DFS search tree with pruning */
    bool searchTree(const QVector<QVector<int>>& adj1,
                    const QVector<QVector<int>>& adj2,
                    const SearchNode& node1, const SearchNode& node2,
                    QVector<int>& mapping, int& explored);

    /** @brief Check color compatibility between two colorings */
    bool colorCompatible(const QVector<int>& c1, const QVector<int>& c2, int n) const;

    /** @brief Compute degree sequence for quick rejection */
    QVector<int> degreeSequence(const QVector<QVector<int>>& adj) const;
};
