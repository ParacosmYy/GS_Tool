/**
 * @file TransitiveClosure8.h
 * @brief 传递闭包(Purdom算法与强连通分量压缩的环感知可达性) — Transitive Closure with Purdom's Algorithm and Strongly Connected Component Condensation for Cycle-aware Reachability
 *
 * 功能: 实现传递闭包(transitive closure)，采用Purdom算法(Purdom's algorithm)
 *       与强连通分量压缩(strongly connected component condensation)实现环感知可达性(cycle-aware reachability)。
 *
 * 协作: TopologicalSort7(拓扑排序) / FloydWarshall6(Floyd-Warshall) / TarjanSCC5(Tarjan SCC)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 传递闭包(Purdom算法与强连通分量压缩)
 */
class TransitiveClosure8 : public QObject {
    Q_OBJECT

public:
    /** @brief Closure computation result */
    struct ClosureResult {
        QVector<QVector<bool>> reachability;
        int numSCCs = 0;
        QVector<int> sccIds;
        int numReachablePairs = 0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numVertices = 0;
        int numEdges = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit TransitiveClosure8(QObject *parent = nullptr);
    ~TransitiveClosure8() override;

    /** @brief Compute transitive closure using Purdom's SCC condensation */
    ClosureResult compute(const QVector<QVector<int>>& adjacency, int n);

    /** @brief Check if vertex u can reach vertex v */
    bool canReach(int u, int v) const;

    /** @brief Get all vertices reachable from u */
    QVector<int> reachableFrom(int u) const;

    /** @brief Find SCCs using Tarjan's algorithm */
    QVector<QVector<int>> findSCCs(const QVector<QVector<int>>& adj, int n) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void closureDone(int n, int numSCCs, int reachablePairs, double timeMs);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    QVector<QVector<bool>> m_closure;
    int m_n = 0;

    /** @brief Tarjan's SCC DFS */
    void tarjanDFS(int u, const QVector<QVector<int>>& adj,
                    QVector<int>& disc, QVector<int>& low,
                    QVector<bool>& onStack, QVector<int>& stack,
                    QVector<int>& sccId, int& index, int& sccCount) const;

    /** @brief Topological sort of condensation DAG */
    QVector<int> topoSort(const QVector<QVector<int>>& dagAdj, int n) const;

    /** @brief Forward DFS from a source vertex */
    void forwardReach(int src, const QVector<QVector<int>>& dagAdj,
                       QVector<bool>& reached) const;
};
