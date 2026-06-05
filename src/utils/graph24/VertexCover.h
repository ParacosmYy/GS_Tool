/**
 * @file VertexCover.h
 * @brief 最小顶点覆盖近似算法 — 2-近似保证的无向图顶点覆盖
 *
 * 功能: 实现经典2-近似最小顶点覆盖算法，支持邻接表建图、
 *       顶点覆盖求解、覆盖验证。适用于通信网络拓扑分析、
 *       串口设备连接图的监控节点选取。
 *
 * 协作: DataCorrelator(相关性图) / AnomalyDetector(异常覆盖)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>
#include <QSet>

/**
 * @brief 最小顶点覆盖近似求解器 — 2-近似保证
 */
class VertexCover : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalGraphsBuilt = 0;           ///< 累计建图次数
        int totalCoversComputed = 0;        ///< 累计求解次数
        double avgProcessingTimeMs = 0.0;   ///< 平均处理时间(ms)
    };

    /** @brief 图的边 */
    struct Edge {
        int u = 0;      ///< 顶点u
        int v = 0;      ///< 顶点v
    };

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit VertexCover(QObject* parent = nullptr);

    /**
     * @brief 从边列表建图
     * @param edges 边列表
     * @param vertexCount 顶点数量(0表示自动推断)
     */
    void buildFromEdges(const QList<Edge>& edges, int vertexCount = 0);

    /**
     * @brief 添加一条边
     * @param u 顶点u
     * @param v 顶点v
     */
    void addEdge(int u, int v);

    /**
     * @brief 计算2-近似最小顶点覆盖
     * @return 被选中的顶点集合
     */
    QSet<int> computeApproximateCover();

    /**
     * @brief 带权重的贪心顶点覆盖(选择度数/权重比最大的顶点)
     * @param weights 顶点权重数组
     * @return 被选中的顶点集合
     */
    QSet<int> computeWeightedCover(const QVector<double>& weights);

    /**
     * @brief 验证顶点集合是否为合法覆盖
     * @param cover 候选覆盖集
     * @return 是否覆盖所有边
     */
    bool verifyCover(const QSet<int>& cover) const;

    /**
     * @brief 获取当前图的边列表
     * @return 边列表
     */
    QList<Edge> edges() const;

    /**
     * @brief 获取顶点数量
     * @return 顶点数
     */
    int vertexCount() const;

    /**
     * @brief 获取边数量
     * @return 边数
     */
    int edgeCount() const;

    /**
     * @brief 清空图
     */
    void clear();

    /**
     * @brief 获取统计信息
     * @return 统计引用
     */
    const Stats& stats() const { return m_stats; }

    /**
     * @brief 重置统计信息
     */
    void resetStatistics();

signals:
    /**
     * @brief 覆盖计算完成
     * @param coverSize 覆盖集大小
     * @param edgeCount 边总数
     */
    void coverComputed(int coverSize, int edgeCount);

private:
    QVector<QList<int>> m_adjList;      ///< 邻接表
    QList<Edge> m_edges;                ///< 边列表
    int m_vertexCount;                  ///< 顶点数

    Stats m_stats;
    double m_timeSum = 0.0;             ///< 处理时间累加器
};
