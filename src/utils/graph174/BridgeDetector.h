/**
 * @file BridgeDetector.h
 * @brief 桥边检测器(Tarjan) — Bridge & Articulation Point Detector
 *
 * 功能: 基于Tarjan DFS的无向图桥边和割点检测，单次遍历O(V+E)。
 *       支持邻接表输入，输出所有桥边和割点列表。
 *
 * 协作: BiconnectedComponent(双连通分量) / StronglyConnected(强连通)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 无向图桥边与割点检测器，基于Tarjan算法
 */
class BridgeDetector : public QObject {
    Q_OBJECT

public:
    /** @brief 图的边 */
    using Edge = QPair<int, int>;

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalSearches = 0;          ///< 累计搜索次数
        quint64 bridgesFound = 0;           ///< 累计发现桥边数
        quint64 articulationPointsFound = 0;///< 累计发现割点数
        double avgProcessingTimeMs = 0.0;   ///< 平均处理耗时(ms)
    };

    explicit BridgeDetector(QObject* parent = nullptr);

    /**
     * @brief 从边列表构建邻接表
     * @param edges 边列表
     * @param vertexCount 顶点数
     */
    void buildGraph(const QVector<Edge>& edges, int vertexCount);

    /**
     * @brief 添加单条边(无向)
     * @param u 顶点u
     * @param v 顶点v
     */
    void addEdge(int u, int v);

    /**
     * @brief 检测所有桥边
     * @return 桥边列表
     */
    QVector<Edge> findBridges();

    /**
     * @brief 检测所有割点(关节点)
     * @return 割点索引列表
     */
    QVector<int> findArticulationPoints();

    /**
     * @brief 同时检测桥边和割点
     * @param[out] bridges 桥边列表
     * @param[out] articulationPoints 割点列表
     */
    void findAll(QVector<Edge>& bridges, QVector<int>& articulationPoints);

    /** @brief 获取邻接表(只读) */
    const QVector<QVector<int>>& adjacency() const { return m_adj; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 检测完成 @param bridges 桥边数 @param artPoints 割点数 */
    void detectionCompleted(int bridges, int artPoints);

private:
    /** @brief Tarjan DFS递归 */
    void dfs(int u, int parent, QVector<int>& disc, QVector<int>& low,
             QVector<bool>& visited, int& timer,
             QVector<Edge>* bridges, QVector<int>* artPoints);

    /** @brief 确保邻接表足够大 */
    void ensureSize(int vertex);

    QVector<QVector<int>> m_adj;  ///< 邻接表

    Stats m_stats;
    double m_timeSum = 0.0;
};
