/**
 * @file StronglyConnected.h
 * @brief 强连通分量(Tarjan算法)
 */

#pragma once

#include <QObject>
#include <QVector>

/**
 * @class StronglyConnected
 * @brief Tarjan强连通分量算法 — 线性时间查找有向图的所有强连通分量
 *
 * 基于DFS和Low-link值，一次遍历识别所有SCC。
 * 适用于依赖分析、双向可达性检测、图简化等场景。
 */
class StronglyConnected : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalSearches = 0;      /**< 总搜索次数 */
        int totalComponentsFound = 0; /**< 总发现的SCC数 */
        double avgProcessingTimeMs = 0.0; /**< 平均处理时间(ms) */
    };

    /** @brief 构造函数 */
    explicit StronglyConnected(QObject* parent = nullptr);

    /**
     * @brief 设置图的邻接表
     * @param adj 邻接表: adj[i] = {i的邻居列表}
     * @param numNodes 节点数
     */
    void setGraph(const QVector<QVector<int>>& adj, int numNodes);

    /**
     * @brief 查找所有强连通分量
     * @return SCC列表，每个SCC是节点ID的集合
     */
    QVector<QVector<int>> findComponents();

    /**
     * @brief 检查两点是否在同一SCC中
     * @param u 节点u
     * @param v 节点v
     * @return 是否强连通
     */
    bool isStronglyConnected(int u, int v) const;

    /**
     * @brief 获取节点所属SCC编号
     * @param node 节点ID
     * @return SCC编号(-1表示未计算)
     */
    int componentId(int node) const;

    /** @brief 获取SCC数量 */
    int componentCount() const;

    /** @brief 构建SCC的缩略图(Condensation DAG) */
    QVector<QVector<int>> condensationDAG() const;

    /** @brief 获取统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 搜索完成信号 */
    void searchCompleted(int componentCount);

private:
    void tarjanDFS(int v);

    QVector<QVector<int>> m_adj;   /**< 邻接表 */
    int m_numNodes;                 /**< 节点数 */

    QVector<int> m_index;           /**< DFS序号 */
    QVector<int> m_lowlink;         /**< Low-link值 */
    QVector<bool> m_onStack;        /**< 是否在栈上 */
    QVector<int> m_stack;           /**< DFS栈 */
    QVector<int> m_compId;          /**< 节点→SCC编号映射 */
    int m_currentIndex;             /**< 当前DFS序号 */
    int m_compCount;                /**< SCC数量 */

    QVector<QVector<int>> m_components; /**< SCC列表 */

    Stats m_stats;
    double m_timeSum;
};
