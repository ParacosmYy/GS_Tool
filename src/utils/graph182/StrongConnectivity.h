/**
 * @file StrongConnectivity.h
 * @brief 强连通分量(Tarjan算法+Kosaraju双遍DFS+分量DAG) — Tarjan Strong Connectivity with Kosaraju's Two-Pass DFS and Component DAG
 *
 * 功能: 实现有向图强连通分量(SCC)检测，支持Tarjan单次遍历和Kosaraju双遍DFS，
 *       提取分量DAG(缩点图)、拓扑排序和分量大小统计。
 *
 * 协作: ShortestPath(最短路) / MinSpanningTree(最小生成树) / GraphColoring(图着色)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 强连通分量检测器
 */
class StrongConnectivity : public QObject {
    Q_OBJECT

public:
    /** @brief SCC检测结果 */
    struct SCCResult {
        QVector<QVector<int>> components;  ///< 各强连通分量(节点列表)
        QVector<int> componentId;           ///< 每个节点所属分量编号
        QVector<QPair<int, int>> dagEdges;  ///< 分量DAG的边
        int componentCount = 0;             ///< 分量总数
    };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalRuns = 0;            ///< 累计运行次数
        double avgProcessingTimeMs = 0.0; ///< 平均耗时(ms)
        int lastNodeCount = 0;            ///< 最近节点数
        int lastEdgeCount = 0;            ///< 最近边数
        int lastComponentCount = 0;       ///< 最近分量数
    };

    explicit StrongConnectivity(QObject *parent = nullptr);
    ~StrongConnectivity() override;

    /**
     * @brief Tarjan算法求SCC
     * @param adj 邻接表(adj[u] = u的邻居列表)
     * @return SCC结果
     */
    SCCResult tarjan(const QVector<QVector<int>>& adj);

    /**
     * @brief Kosaraju算法求SCC(双遍DFS)
     * @param adj 邻接表
     * @return SCC结果
     */
    SCCResult kosaraju(const QVector<QVector<int>>& adj);

    /** @brief 从SCC结果提取分量DAG */
    static QVector<QVector<int>> buildComponentDAG(const SCCResult& result,
                                                    int nodeCount);

    /** @brief 分量拓扑排序 */
    static QVector<int> topologicalOrder(const QVector<QVector<int>>& dag);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief SCC检测完成 @param sccCount 分量数 */
    void connectivityCompleted(int sccCount);

private:
    /** @brief Tarjan DFS递归 */
    void tarjanDFS(int u, const QVector<QVector<int>>& adj,
                   QVector<int>& disc, QVector<int>& low,
                   QVector<bool>& onStack, QVector<int>& stack,
                   int& index, SCCResult& result);

    /** @brief Kosaraju第一遍DFS(后序) */
    void kosarajuDFS1(int u, const QVector<QVector<int>>& adj,
                      QVector<bool>& visited, QVector<int>& order);

    /** @brief Kosaraju第二遍DFS(逆图) */
    void kosarajuDFS2(int u, const QVector<QVector<int>>& radj,
                      QVector<bool>& visited, QVector<int>& comp,
                      int compId);

    Stats m_stats;
    double m_timeSum = 0.0;
};
