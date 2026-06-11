/**
 * @file MaximumClique9.h
 * @brief 最大团(分支定界顶点着色与度数剪枝实现精确最大团枚举) — Maximum Clique with Branch-and-bound Vertex Coloring and Degree-based Pruning for Exact Maximum Clique Enumeration
 *
 * 功能: 实现最大团(maximum clique)，采用分支定界顶点着色(branch-and-bound vertex coloring)
 *       与度数剪枝(degree-based pruning)实现精确最大团枚举(exact maximum clique enumeration)。
 *
 * 协作: GraphColoring(图着色) / BronKerbosch(BK算法) / CommunityDetection(社区发现)
 */
#pragma once

#include <QObject>
#include <QVector>

class MaximumClique9 : public QObject {
    Q_OBJECT

public:
    /** @brief Clique search result */
    struct CliqueResult {
        QVector<int> maxClique;
        int maxCliqueSize = 0;
        int nodesExplored = 0;
        int colorCalls = 0;
        double searchTimeMs = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalSearches = 0;
        int lastGraphSize = 0;
        int largestCliqueFound = 0;
        double avgSearchTimeMs = 0.0;
    };

    explicit MaximumClique9(QObject *parent = nullptr);
    ~MaximumClique9() override;

    /** @brief Find maximum clique in graph given as adjacency matrix */
    CliqueResult findMaximumClique(const QVector<QVector<int>>& adjMatrix);

    /** @brief Find maximum clique given as adjacency lists */
    CliqueResult findMaximumCliqueList(const QVector<QVector<int>>& adjLists);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void searchDone(int n, int cliqueSize, double timeMs);

private:
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Best clique found so far */
    QVector<int> m_bestClique;
    int m_bestSize = 0;

    /** @brief Search statistics for current run */
    int m_nodesExplored = 0;
    int m_colorCalls = 0;

    /** @brief Greedy graph coloring for upper bound */
    QVector<int> greedyColor(const QVector<QVector<int>>& adj,
                              const QVector<int>& candidates) const;

    /** @brief Recursive branch-and-bound search */
    void expand(const QVector<QVector<int>>& adj,
                QVector<int>& current,
                QVector<int>& candidates);

    /** @brief Degree-based vertex ordering (descending) */
    QVector<int> degreeOrder(const QVector<QVector<int>>& adj) const;
};
