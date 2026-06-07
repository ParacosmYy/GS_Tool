/**
 * @file VertexCover5.h
 * @brief 顶点覆盖(冠分解+LP松弛+NPR约简) — Vertex Cover via Crown Decomposition with LP Relaxation and Nodal Point Reduction
 *
 * 功能: 实现基于冠分解的顶点覆盖算法，支持LP松弛求解、
 *       节点约简(NPR)和半顶点近似保证。
 *
 * 协作: GraphColor5(图着色) / MaxFlow5(最大流) / BipartiteMatch4(二分匹配)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 顶点覆盖(冠分解+LP松弛+NPR约简)
 */
class VertexCover5 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalSolves = 0;
        int numVertices = 0;
        int numEdges = 0;
        int coverSize = 0;
        int crownReductions = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit VertexCover5(QObject *parent = nullptr);
    ~VertexCover5() override;

    /** @brief Solve vertex cover on adjacency list graph */
    QVector<int> solve(const QVector<QVector<int>>& adjList);

    /** @brief Compute LP relaxation (half-integral solution) */
    QVector<double> lpRelaxation(const QVector<QVector<int>>& adjList) const;

    /** @brief Find crown decomposition (crown, head, rest) */
    bool findCrown(const QVector<QVector<int>>& adj,
                   QVector<int>& crown, QVector<int>& head) const;

    /** @brief NPR: nodal point reduction via degree-1 folding */
    QVector<QVector<int>> nodalReduce(
        const QVector<QVector<int>>& adj,
        QVector<int>& mapping) const;

    /** @brief Verify vertex cover correctness */
    bool verifyCover(const QVector<QVector<int>>& adj,
                     const QVector<int>& cover) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void coverFound(int size, int reductions, double timeMs);

private:
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Greedy 2-approximation as fallback */
    QVector<int> greedyApprox(const QVector<QVector<int>>& adj) const;

    /** @brief Maximum matching via augmenting paths (bipartite) */
    QVector<int> maxMatching(const QVector<QVector<int>>& adj) const;

    /** @brief Build complement of vertex set */
    QVector<int> complement(const QVector<int>& set, int n) const;

    /** @brief Find maximum independent set via crown */
    QVector<int> crownIndependentSet(
        const QVector<QVector<int>>& adj) const;
};
