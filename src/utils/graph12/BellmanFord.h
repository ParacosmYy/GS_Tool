/**
 * @file BellmanFord.h
 * @brief Bellman-Ford最短路径 — 支持负权边检测
 *
 * 单源最短路径算法, 可处理负权边并检测负权环。
 * 时间复杂度O(VE), 适用于含负权的稀疏图。
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>
#include <QMap>

/**
 * @class BellmanFord
 * @brief Bellman-Ford最短路径 — 负权边+负权环检测
 *
 * 支持添加有向/无向带权边, 返回最短距离和路径,
 * 可检测并报告负权环。
 */
class BellmanFord : public QObject
{
    Q_OBJECT

public:
    /** @brief 边结构 */
    struct Edge {
        int from = 0;     ///< 起始顶点
        int to = 0;       ///< 目标顶点
        double weight = 0.0; ///< 边权重
    };

    /** @brief 统计信息 */
    struct Stats {
        quint64 totalQueries = 0;      ///< 总查询次数
        quint64 totalEdgesProcessed = 0; ///< 总处理边数
        double  avgProcessingTimeMs = 0.0; ///< 平均耗时(ms)
    };

    /** @brief 构造函数 @param parent 父对象 */
    explicit BellmanFord(QObject* parent = nullptr);

    /**
     * @brief 设置顶点数
     * @param numVertices 顶点数
     */
    void setVertexCount(int numVertices);

    /**
     * @brief 添加有向边
     * @param from 起点
     * @param to 终点
     * @param weight 权重
     */
    void addEdge(int from, int to, double weight);

    /**
     * @brief 添加无向边(双向)
     * @param u 端点1
     * @param v 端点2
     * @param weight 权重
     */
    void addUndirectedEdge(int u, int v, double weight);

    /**
     * @brief 计算从源点到所有顶点的最短路径
     * @param source 源点
     * @return pair: (距离数组, 是否存在负权环)
     */
    QPair<QVector<double>, bool> computeShortestPaths(int source) const;

    /**
     * @brief 计算从源点到目标点的最短路径
     * @param source 起点
     * @param target 终点
     * @return pair: (最短距离, 路径顶点列表); 不可达时距离=INF
     */
    QPair<double, QVector<int>> computePath(int source, int target) const;

    /**
     * @brief 检测图中是否存在负权环
     * @return true表示存在负权环
     */
    bool hasNegativeCycle() const;

    /**
     * @brief 获取负权环中的顶点(若存在)
     * @return 负权环顶点列表
     */
    QVector<int> findNegativeCycle() const;

    /** @brief 获取所有边 */
    QVector<Edge> edges() const;

    /** @brief 获取顶点数 */
    int vertexCount() const;

    /** @brief 获取统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

    /** @brief 无穷大标记 */
    static constexpr double INF = 1e18;

signals:
    /** @brief 查询完成 @param source 源点 @param hasNegCycle 是否有负权环 */
    void queryCompleted(int source, bool hasNegCycle);

private:
    int m_numVertices = 0;    ///< 顶点数
    QVector<Edge> m_edges;    ///< 边列表
    mutable Stats m_stats;    ///< 操作统计
    mutable double m_timeSum = 0.0; ///< 累计耗时
};
