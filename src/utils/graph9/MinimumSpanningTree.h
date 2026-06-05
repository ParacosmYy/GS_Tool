/**
 * @file MinimumSpanningTree.h
 * @brief 最小生成树 — Prim/Kruskal算法
 *
 * 功能: 对无向加权图求解最小生成树，提供Prim算法(基于优先队列)
 *       和Kruskal算法(基于并查集)，返回MST的边集和总权重。
 *
 * 协作: LuDecomposition(图分析) / ShortestPath(最短路径)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 最小生成树求解器
 */
class MinimumSpanningTree : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalSearches = 0;          ///< 累计搜索次数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理耗时(ms)
    };

    /** @brief 构造函数 @param parent 父对象 */
    explicit MinimumSpanningTree(QObject* parent = nullptr);

    /**
     * @brief 设置图(邻接表)
     * @param numNodes 节点数
     * @param adj 邻接表 adj[u] = [(v, weight), ...]
     */
    void setGraph(int numNodes,
                  const QVector<QVector<QPair<int, double>>>& adj);

    /**
     * @brief Prim算法求MST
     * @return MST边集 [(u, v), ...]
     */
    QVector<QPair<int, int>> prim();

    /**
     * @brief Kruskal算法求MST
     * @return MST边集 [(u, v), ...]
     */
    QVector<QPair<int, int>> kruskal();

    /**
     * @brief 获取MST总权重(最近一次计算)
     * @return 总权重
     */
    double totalWeight() const;

    /** @brief 获取统计 @return 统计信息常引用 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief MST计算完成 @param algorithm 算法名 @param edgeCount 边数 */
    void mstComputed(const QString& algorithm, int edgeCount);

private:
    /**
     * @brief 并查集查找(带路径压缩)
     * @param parent 父节点数组
     * @param x 查找节点
     * @return 根节点
     */
    int findRoot(QVector<int>& parent, int x) const;

    /**
     * @brief 并查集合并(按秩)
     * @param parent 父节点数组
     * @param rank 秩数组
     * @param x 节点x
     * @param y 节点y
     * @return 是否合并成功(不在同一集合)
     */
    bool unionSet(QVector<int>& parent, QVector<int>& rank,
                  int x, int y) const;

    int m_numNodes;                                     ///< 节点数
    QVector<QVector<QPair<int, double>>> m_adj;         ///< 邻接表
    double m_totalWeight;                               ///< 最近MST总权重
    Stats m_stats;                                      ///< 统计信息
    double m_timeSum;                                   ///< 累计耗时
};
