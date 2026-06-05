/**
 * @file KruskalMST.h
 * @brief Kruskal最小生成树算法 — 贪心+并查集
 *
 * 功能: 计算无向图的最小生成树(MST)，使用Kruskal贪心算法
 *       配合并查集实现。时间复杂度O(E log E)。
 *
 * 协作: DisjointSet(并查集) / PrimMST(Prim算法)
 */
#ifndef KRUSKALMST_H
#define KRUSKALMST_H

#include <QObject>
#include <QVector>

/**
 * @brief Kruskal最小生成树
 */
class KruskalMST : public QObject {
    Q_OBJECT

public:
    /** @brief 边 */
    struct Edge {
        int from = 0;        ///< 起点
        int to = 0;          ///< 终点
        double weight = 0.0; ///< 权重
    };

    /** @brief 统计信息 */
    struct Stats {
        quint64 totalQueries = 0;        ///< 累计查询次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    /** @brief MST结果 */
    struct MstResult {
        QVector<Edge> edges;             ///< MST中的边
        double totalWeight = 0.0;        ///< MST总权重
        bool connected = false;          ///< 图是否连通
    };

    explicit KruskalMST(QObject* parent = nullptr);

    /** @brief 计算最小生成树
     *  @param vertexCount 顶点数
     *  @param edges 边列表
     *  @return MST结果 */
    MstResult compute(int vertexCount, const QVector<Edge>& edges);

    /** @brief 获取统计 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 计算完成 @param edgeCount MST边数 @param totalWeight 总权重 */
    void mstCompleted(int edgeCount, double totalWeight);

private:
    /** @brief 并查集查找(路径压缩) */
    int findParent(QVector<int>& parent, int x);

    /** @brief 并查集合并(按秩) */
    bool unionSets(QVector<int>& parent, QVector<int>& rank,
                   int x, int y);

    double m_timeSum;   ///< 处理时间累加器
    Stats m_stats;      ///< 统计信息
};

#endif // KRUSKALMST_H
