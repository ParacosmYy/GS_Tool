/**
 * @file FloydWarshall.h
 * @brief Floyd-Warshall全源最短路径算法
 *
 * 功能: 计算所有顶点对之间的最短路径，支持负权边，
 *       可检测负权环。时间复杂度O(V^3)，空间复杂度O(V^2)。
 *
 * 协作: BellmanFord(单源负权) / DijkstraShortestPath(单源非负权)
 */
#ifndef FLOYDWARSHALL_H
#define FLOYDWARSHALL_H

#include <QObject>
#include <QVector>

/**
 * @brief Floyd-Warshall全源最短路径
 */
class FloydWarshall : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalQueries = 0;        ///< 累计查询次数
        quint64 negativeCyclesFound = 0; ///< 发现负权环次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    /** @brief 查询结果 */
    struct AllPairsResult {
        QVector<QVector<double>> distances; ///< 距离矩阵
        QVector<QVector<int>> next;         ///< 路径重建矩阵
        bool hasNegativeCycle = false;      ///< 是否有负权环
    };

    explicit FloydWarshall(QObject* parent = nullptr);

    /** @brief 计算全源最短路径
     *  @param adjacency 邻接矩阵(inf表示无直接边)
     *  @return 全源结果 */
    AllPairsResult allPairsShortestPaths(
        const QVector<QVector<double>>& adjacency);

    /** @brief 回溯最短路径
     *  @param result 查询结果
     *  @param from 起点
     *  @param to 终点
     *  @return 路径节点序列 */
    QVector<int> reconstructPath(const AllPairsResult& result,
                                 int from, int to) const;

    /** @brief 检测负权环
     *  @param adjacency 邻接矩阵
     *  @return 是否存在负权环 */
    bool hasNegativeCycle(
        const QVector<QVector<double>>& adjacency);

    /** @brief 获取统计 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 查询完成 @param n 顶点数 @param hasNegCycle 负权环 */
    void queryCompleted(int n, bool hasNegCycle);

private:
    double m_timeSum;   ///< 处理时间累加器
    Stats m_stats;      ///< 统计信息
};

#endif // FLOYDWARSHALL_H
