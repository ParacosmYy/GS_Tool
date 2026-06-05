/**
 * @file PrimMST.h
 * @brief Prim最小生成树算法 — 优先队列实现
 *
 * 功能: 计算无向图的最小生成树，使用Prim算法配合优先队列。
 *       适合稠密图，时间复杂度O(E log V)。
 *
 * 协作: KruskalMST(稀疏图) / FibonacciHeap(优先队列)
 */
#ifndef PRIMMST_H
#define PRIMMST_H

#include <QObject>
#include <QVector>

/**
 * @brief Prim最小生成树
 */
class PrimMST : public QObject {
    Q_OBJECT

public:
    /** @brief 邻接表中的边 */
    struct AdjEdge {
        int to = 0;          ///< 目标顶点
        double weight = 0.0; ///< 权重
    };

    /** @brief 统计信息 */
    struct Stats {
        quint64 totalQueries = 0;        ///< 累计查询次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    /** @brief MST结果 */
    struct MstResult {
        QVector<QPair<int, int>> edges;  ///< MST中的边(顶点对)
        QVector<double> weights;         ///< 对应边权重
        double totalWeight = 0.0;        ///< MST总权重
    };

    explicit PrimMST(QObject* parent = nullptr);

    /** @brief 从邻接矩阵计算MST
     *  @param adjacency 邻接矩阵(-1表示无直接边)
     *  @return MST结果 */
    MstResult computeFromMatrix(
        const QVector<QVector<double>>& adjacency);

    /** @brief 从邻接表计算MST
     *  @param vertexCount 顶点数
     *  @param adjList 邻接表
     *  @return MST结果 */
    MstResult computeFromList(
        int vertexCount,
        const QVector<QVector<AdjEdge>>& adjList);

    /** @brief 获取统计 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 计算完成 @param edgeCount MST边数 @param totalWeight 总权重 */
    void mstCompleted(int edgeCount, double totalWeight);

private:
    double m_timeSum;   ///< 处理时间累加器
    Stats m_stats;      ///< 统计信息
};

#endif // PRIMMST_H
