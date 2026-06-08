/**
 * @file VertexCover7.h
 * @brief 顶点覆盖(半整数LP松弛+冠分解核化参数化约减) — Vertex Cover with Half-integrality LP Relaxation and Crown Decomposition Kernel for Parameterized Reduction
 *
 * 功能: 实现顶点覆盖(Vertex Cover)算法，使用半整数性LP松弛(half-integrality LP relaxation)，
 *       结合冠分解(crown decomposition)核化(kernelization)进行参数化约减(parameterized reduction)。
 *
 * 协作: MaxClique5(最大团) / GraphColoring3(图着色) / BipartiteMatch4(二部匹配)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 顶点覆盖(半整数LP松弛+冠分解核化)
 */
class VertexCover7 : public QObject {
    Q_OBJECT

public:
    /** @brief Edge representation */
    struct Edge {
        int u = 0;
        int v = 0;
    };

    /** @brief LP relaxation result */
    struct LPResult {
        QVector<double> fractional;  // half-integral values in {0, 0.5, 1}
        double objective = 0.0;
        bool optimal = false;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numVertices = 0;
        int numEdges = 0;
        int coverSize = 0;
        int kernelSize = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit VertexCover7(QObject *parent = nullptr);
    ~VertexCover7() override;

    /** @brief Set graph: number of vertices and edges */
    void setGraph(int numVertices, const QVector<Edge>& edges);

    /** @brief Solve vertex cover via LP relaxation + crown kernelization */
    QVector<int> solve(int parameter = -1);

    /** @brief Get the LP relaxation solution */
    LPResult lpRelaxation() const;

    /** @brief Apply crown decomposition kernelization, return reduced graph info */
    int crownKernelize(QVector<int>& forcedVertices, QVector<int>& removedVertices);

    /** @brief Verify if given vertex set is a valid cover */
    bool verifyCover(const QVector<int>& cover) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveCompleted(int coverSize, int kernelSize, double timeMs);
    void kernelReduced(int originalVertices, int reducedVertices);

private:
    int m_numVertices = 0;
    QVector<Edge> m_edges;
    QVector<QVector<int>> m_adj;  // adjacency list

    LPResult m_lpResult;
    QVector<int> m_cover;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build adjacency list from edge list */
    void buildAdjacency();

    /** @brief Solve half-integrality LP relaxation (Nemhauser-Trotter) */
    LPResult solveHalfIntegralLP() const;

    /** @brief Find crown decomposition: crown C, head H, rest R */
    bool findCrown(QVector<int>& crown, QVector<int>& head) const;

    /** @brief Find maximum matching using augmenting paths */
    QVector<int> maxMatching(const QVector<QVector<int>>& bipAdj,
                              int leftSize, int rightSize) const;

    /** @brief Bounded search tree for parameterized vertex cover */
    bool boundedSearch(int k, QVector<int>& currentCover,
                       const QVector<Edge>& remaining);

    /** @brief Select edge not covered by current partial cover */
    int selectUncoveredEdge(const QVector<int>& partialCover,
                            const QVector<Edge>& edges) const;
};
