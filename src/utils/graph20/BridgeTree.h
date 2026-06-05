/**
 * @file BridgeTree.h
 * @brief 桥树(Bridge Tree) — 二边连通分量分解
 *
 * 功能: 实现桥树(Bridge Tree)数据结构，将无向图分解为
 *       二边连通分量(2-edge-connected components)，构建分量树。
 *       支持动态添加边、查询桥边、分量归属和路径桥边计数。
 *       适用于网络拓扑分析和通信链路可靠性评估。
 *
 * 协作: UnionFind(连通性) / TopologicalSort(依赖分析)
 */
#pragma once

#include <QObject>
#include <QVector>

#include <utility>
#include <vector>

/**
 * @brief 桥树 — 无向图的二边连通分量分解
 */
class BridgeTree : public QObject {
    Q_OBJECT

public:
    /** @brief 边信息 */
    struct Edge {
        int from;               ///< 起点
        int to;                 ///< 终点
        int edgeId;             ///< 边编号
    };

    /** @brief 统计信息 */
    struct Stats {
        int totalQueries = 0;           ///< 累计查询次数
        int totalEdgeAdditions = 0;     ///< 累计添加边次数
        int totalRebuilds = 0;          ///< 累计重建次数
        int totalComponents = 0;        ///< 当前分量数
        int totalBridges = 0;           ///< 当前桥数
        double avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    /**
     * @brief 构造函数
     * @param numVertices 顶点数
     * @param parent 父对象
     */
    explicit BridgeTree(int numVertices = 0, QObject* parent = nullptr);

    /**
     * @brief 添加无向边
     * @param u 顶点u
     * @param v 顶点v
     */
    void addEdge(int u, int v);

    /**
     * @brief 执行桥分解(DFS Tarjan算法)
     */
    void build();

    /**
     * @brief 查询某条边是否为桥
     * @param u 顶点u
     * @param v 顶点v
     * @return true=是桥
     */
    bool isBridge(int u, int v) const;

    /**
     * @brief 查询顶点所属分量编号
     * @param v 顶点
     * @return 分量编号(-1=未分配)
     */
    int componentOf(int v) const;

    /**
     * @brief 获取所有桥边
     * @return 桥边列表
     */
    QVector<std::pair<int, int>> bridges() const;

    /**
     * @brief 获取分量中的所有顶点
     * @param compId 分量编号
     * @return 顶点列表
     */
    QVector<int> componentVertices(int compId) const;

    /**
     * @brief 计算两顶点间路径上的桥数
     * @param u 起点
     * @param v 终点
     * @return 桥的数量(-1=不可达)
     */
    int bridgesOnPath(int u, int v) const;

    /**
     * @brief 检查两顶点是否在同一二边连通分量
     * @param u 顶点u
     * @param v 顶点v
     * @return true=同一分量
     */
    bool sameComponent(int u, int v) const;

    /** @brief 获取顶点数 @return 顶点数 */
    int vertexCount() const { return m_n; }

    /** @brief 获取分量数 @return 分量数 */
    int componentCount() const { return m_componentCount; }

    /** @brief 获取统计信息 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计信息 */
    void resetStatistics();

    /** @brief 重置图结构 @param numVertices 新的顶点数 */
    void reset(int numVertices);

private:
    /**
     * @brief Tarjan DFS查找桥边
     * @param u 当前顶点
     * @param parentEdgeId 父边编号
     * @param timer 时间戳计数器
     */
    void tarjanDfs(int u, int parentEdgeId, int& timer);

    /**
     * @brief 为连通分量分配编号
     * @param u 当前顶点
     * @param compId 分量编号
     */
    void assignComponents(int u, int compId);

    /**
     * @brief 在桥树上的BFS(LCA预处理)
     * @param root 根节点
     */
    void bfsBridgeTree(int root);

    int m_n;                                    ///< 顶点数
    std::vector<std::vector<Edge>> m_adj;       ///< 邻接表
    std::vector<int> m_disc;                    ///< DFS发现时间
    std::vector<int> m_low;                     ///< 最低可达时间
    std::vector<int> m_component;               ///< 分量归属
    std::vector<bool> m_isBridgeEdge;           ///< 边是否为桥
    int m_componentCount;                       ///< 分量总数
    bool m_built;                               ///< 是否已构建

    /* 桥树结构 */
    std::vector<std::vector<int>> m_bridgeTree; ///< 桥树邻接表
    std::vector<int> m_depth;                   ///< 桥树深度
    std::vector<int> m_parent;                  ///< 桥树父节点
    std::vector<int> m_distFromRoot;            ///< 到根的桥距离

    Stats m_stats;                  ///< 统计信息
    double m_timeSum = 0.0;         ///< 累计耗时
};
