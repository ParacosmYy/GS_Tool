/**
 * @file StrongConnectivity4.h
 * @brief 强连通分量(Tarjan+桥分量树+2SCC凝聚) — Strongly Connected Components via Tarjan with Bridge-Component Tree and 2SCC Condensation
 *
 * 功能: 实现Tarjan强连通分量算法，支持桥分量树构建、
 *       2SCC凝聚图生成和拓扑排序。
 *
 * 协作: Biconnectivity5(双连通分量) / BridgeFinder3(桥检测) / GraphColoring6(图着色)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 强连通分量(Tarjan+桥分量树+2SCC凝聚)
 */
class StrongConnectivity4 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalRuns = 0;
        int numVertices = 0;
        int numEdges = 0;
        int numComponents = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit StrongConnectivity4(QObject *parent = nullptr);
    ~StrongConnectivity4() override;

    /** @brief Find all SCCs using Tarjan's algorithm */
    QVector<QVector<int>> findSCCs(int n, const QVector<QPair<int, int>>& edges);

    /** @brief Build condensation DAG from SCCs */
    QVector<QPair<int, int>> buildCondensation(const QVector<QVector<int>>& sccs,
                                                 const QVector<QPair<int, int>>& edges) const;

    /** @brief Build bridge-component tree */
    QVector<QVector<int>> bridgeComponentTree(const QVector<QVector<int>>& sccs,
                                                const QVector<QPair<int, int>>& edges) const;

    /** @brief Topological sort of condensation */
    QVector<int> topoSort(int numSccs, const QVector<QPair<int, int>>& dagEdges) const;

    /** @brief Check if graph is strongly connected */
    bool isStronglyConnected(int n, const QVector<QPair<int, int>>& edges);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void decompositionCompleted(int components, int vertices, double timeMs);

private:
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Tarjan DFS visit */
    void tarjanDFS(int v, int& index, QVector<int>& idx, QVector<int>& lowLink,
                    QVector<bool>& onStack, QVector<int>& stack,
                    QVector<QVector<int>>& sccs,
                    const QVector<QVector<int>>& adj) const;

    /** @brief Build adjacency list from edge list */
    QVector<QVector<int>> buildAdj(int n, const QVector<QPair<int, int>>& edges) const;
};
