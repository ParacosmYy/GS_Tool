/**
 * @file GraphIsomorphism10.h
 * @brief 图同构(个体化-细化+自同构群检测搜索树剪枝) — Graph Isomorphism with Individualization-Refinement and Search Tree Pruning via Automorphism Group Detection
 *
 * 功能: 实现图同构检测(Graph Isomorphism)，使用个体化-细化范式
 *       (individualization-refinement paradigm)构建搜索树，自同构群
 *       (automorphism group)检测实现搜索树剪枝优化。
 *
 * 协作: MaxClique7(最大团) / GraphColoring8(图着色) / StronglyConnected9(强连通分量)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 图同构(个体化-细化+自同构群剪枝)
 */
class GraphIsomorphism10 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numVertices = 0;
        int numEdges = 0;
        int searchNodesExplored = 0;
        int pruningCuts = 0;
        int automorphismsFound = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit GraphIsomorphism10(QObject *parent = nullptr);
    ~GraphIsomorphism10() override;

    /** @brief Check if two adjacency matrices represent isomorphic graphs */
    bool isIsomorphic(const QVector<QVector<int>>& adj1,
                       const QVector<QVector<int>>& adj2);

    /** @brief Find automorphism generators for a graph */
    QVector<QVector<int>> findAutomorphisms(const QVector<QVector<int>>& adj);

    /** @brief Get the vertex mapping from last isomorphism check */
    QVector<int> mapping() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void searchCompleted(int nodesExplored, int pruningCuts, double timeMs);

private:
    int m_n = 0;
    QVector<QVector<int>> m_adj1;
    QVector<QVector<int>> m_adj2;
    QVector<int> m_mapping;
    QVector<QVector<int>> m_automorphisms;
    bool m_found = false;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Color refinement (Weisfeiler-Lehman) */
    QVector<int> refine(const QVector<QVector<int>>& adj,
                         const QVector<int>& coloring) const;

    /** @brief Individualize vertex v in coloring */
    QVector<int> individualize(const QVector<int>& coloring,
                                int v, int newColor) const;

    /** @brief Search tree node for individualization-refinement */
    struct SearchNode {
        QVector<int> coloring;
        int depth = 0;
        int targetVertex = -1;
    };

    /** @brief Recursive search with automorphism pruning */
    void searchTree(const SearchNode& node);

    /** @brief Check if current coloring is discrete (bijection) */
    bool isDiscrete(const QVector<int>& coloring) const;

    /** @brief Check if permutation preserves adjacency */
    bool isAutomorphism(const QVector<int>& perm,
                         const QVector<QVector<int>>& adj) const;

    /** @brief Select vertex for individualization (target cell) */
    int selectTargetVertex(const QVector<int>& coloring) const;

    /** @brief Count edges in adjacency matrix */
    static int countEdges(const QVector<QVector<int>>& adj);

    /** @brief Compute degree sequence */
    QVector<int> degreeSequence(const QVector<QVector<int>>& adj) const;
};
