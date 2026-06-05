/**
 * @file BellmanFord.h
 * @brief Bellman-Ford最短路径 — 支持负权边和负权环检测
 *
 * 功能: 支持动态添加边，计算单源最短路径，
 *       可检测负权环。时间复杂度O(VE)。
 *
 * 协作: DijkstraShortestPath(非负权) / FloydWarshall(全源)
 */
#ifndef BELLMANFORD_H
#define BELLMANFORD_H

#include <QObject>
#include <QVector>

/**
 * @brief Bellman-Ford最短路径算法
 */
class BellmanFord : public QObject {
    Q_OBJECT

public:
    /** @brief 边结构 */
    struct Edge {
        int from = 0;         ///< 起点
        int to = 0;           ///< 终点
        double weight = 0.0;  ///< 权重
    };

    /** @brief 统计信息 */
    struct Stats {
        quint64 totalQueries = 0;       ///< 累计查询次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    explicit BellmanFord(QObject* parent = nullptr);

    /** @brief 添加有向边
     *  @param from 起点 @param to 终点 @param weight 权重 */
    void addEdge(int from, int to, double weight);

    /** @brief 计算最短路径
     *  @param source 起点 @param target 终点
     *  @return 路径节点序列(空=不可达/有负权环) */
    QVector<int> shortestPath(int source, int target);

    /** @brief 检测图中是否有负权环 @return 是否存在负权环 */
    bool hasNegativeCycle();

    /** @brief 设置顶点数 @param n 顶点数 */
    void setVertexCount(int n);

    /** @brief 获取统计 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 查询完成 @param source 起点 @param target 终点 @param pathLen 路径长度 */
    void queryCompleted(int source, int target, int pathLen);

private:
    QVector<Edge> m_edges;  ///< 边列表
    int m_vertexCount;      ///< 顶点数
    double m_timeSum;       ///< 处理时间累加器
    Stats  m_stats;         ///< 统计信息
};

#endif // BELLMANFORD_H
