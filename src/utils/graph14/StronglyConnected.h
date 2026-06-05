/**
 * @file StronglyConnected.h
 * @brief 强连通分量算法 — Tarjan SCC + Kosaraju变体
 *
 * 功能: 在有向图中查找所有强连通分量(SCC)，支持Tarjan算法
 *       和Kosaraju变体，可选输出拓扑排序和缩点DAG。
 *
 * 协作: CycleDetector(环检测) / DataFlowMeter(数据流分析)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>
#include <QMap>

/**
 * @brief 强连通分量 — Tarjan SCC / Kosaraju变体
 */
class StronglyConnected : public QObject {
    Q_OBJECT

public:
    /** @brief 算法选择 */
    enum class Algorithm {
        Tarjan,         ///< Tarjan算法(单次DFS)
        Kosaraju        ///< Kosaraju算法(两次DFS)
    };
    Q_ENUM(Algorithm)

    /** @brief 统计 */
    struct Stats {
        int totalGraphsProcessed = 0;       ///< 累计处理图数
        int totalSccsFound = 0;             ///< 累计SCC数
        int totalVerticesProcessed = 0;     ///< 累计处理顶点数
        int totalEdgesProcessed = 0;        ///< 累计处理边数
        double avgProcessingTimeMs = 0.0;   ///< 平均处理耗时(ms)
    };

    /** @brief SCC结果 */
    struct SccResult {
        QVector<QVector<int>> components;   ///< 各SCC的顶点集合
        QVector<int> componentId;           ///< 每个顶点所属SCC编号
        QVector<QPair<int,int>> dagEdges;   ///< 缩点DAG的边
        int largestComponentSize = 0;       ///< 最大SCC大小
        bool hasCycle = false;              ///< 图中是否存在环
    };

    explicit StronglyConnected(QObject* parent = nullptr);

    /** @brief 设置算法 @param algo 算法 */
    void setAlgorithm(Algorithm algo);

    /**
     * @brief 在邻接表图上查找SCC
     * @param adjList 邻接表(adjList[v] = {v的邻居列表})
     * @return SCC结果
     */
    SccResult findSCC(const QVector<QVector<int>>& adjList);

    /**
     * @brief 在边列表图上查找SCC
     * @param edges 边列表({from, to})
     * @param vertexCount 顶点总数
     * @return SCC结果
     */
    SccResult findSCCFromEdges(const QVector<QPair<int,int>>& edges,
                               int vertexCount);

    /**
     * @brief 判断图中是否有环
     * @param adjList 邻接表
     * @return 有环返回true
     */
    bool hasCycle(const QVector<QVector<int>>& adjList);

    /**
     * @brief 构建缩点DAG
     * @param adjList 原图邻接表
     * @param componentId 每个顶点的SCC编号
     * @param sccCount SCC总数
     * @return DAG邻接表
     */
    QVector<QVector<int>> buildCondensationDAG(
        const QVector<QVector<int>>& adjList,
        const QVector<int>& componentId, int sccCount);

    /**
     * @brief 拓扑排序(基于SCC)
     * @param dagEdges DAG边列表
     * @param nodeCount 节点数(SCC数)
     * @return 拓扑序(逆后序)
     */
    QVector<int> topologicalSort(const QVector<QPair<int,int>>& dagEdges,
                                 int nodeCount);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief SCC查找完成 @param sccCount SCC数量 @param largestSize 最大SCC大小 */
    void sccComplete(int sccCount, int largestSize);

private:
    /** @brief Tarjan算法实现 @param adjList 邻接表 @return SCC结果 */
    SccResult tarjanSCC(const QVector<QVector<int>>& adjList);

    /** @brief Kosaraju算法实现 @param adjList 邻接表 @return SCC结果 */
    SccResult kosarajuSCC(const QVector<QVector<int>>& adjList);

    /** @brief Tarjan DFS @param u 当前顶点 @param adjList 邻接表 @param index 当前编号 @param stack 栈 @param onStack 在栈标记 @param indices 编号数组 @param lowlink 低链接值 @param result 结果 */
    void tarjanDFS(int u, const QVector<QVector<int>>& adjList,
                   int& index, QVector<int>& stack, QVector<bool>& onStack,
                   QVector<int>& indices, QVector<int>& lowlink,
                   QVector<QVector<int>>& result);

    /** @brief Kosaraju逆图DFS(求逆后序) @param u 当前顶点 @param adj 邻接表 @param visited 访问标记 @param order 逆后序 */
    void kosarajuDFS1(int u, const QVector<QVector<int>>& adj,
                      QVector<bool>& visited, QVector<int>& order);

    /** @brief Kosaraju正向DFS(分配SCC) @param u 当前顶点 @param radj 逆邻接表 @param visited 访问标记 @param component 当前SCC */
    void kosarajuDFS2(int u, const QVector<QVector<int>>& radj,
                      QVector<bool>& visited, QVector<int>& component);

    Algorithm m_algorithm = Algorithm::Tarjan;  ///< 算法选择
    Stats m_stats;                              ///< 统计信息
    double m_timeSum = 0.0;                     ///< 累计耗时
};
