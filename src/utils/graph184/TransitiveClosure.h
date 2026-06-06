/**
 * @file TransitiveClosure.h
 * @brief 传递闭包(Floyd-Warshall + Warshall位集优化) — Transitive Closure via Floyd-Warshall and Warshall's Bitset Optimization
 *
 * 功能: 实现有向图的传递闭包，支持Floyd-Warshall O(n³)算法和
 *       Warshall位集优化O(n³/word)算法，可达性查询和路径统计。
 *
 * 协作: StronglyConnected3(强连通分量) / TopologicalSort7(拓扑排序) / GraphBFS5(BFS)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 传递闭包计算器
 */
class TransitiveClosure : public QObject {
    Q_OBJECT

public:
    /** @brief 算法选择 */
    enum Algorithm { FloydWarshall, WarshallBitset };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalRuns = 0;           ///< 累计运行次数
        int lastVertices = 0;            ///< 最近顶点数
        int lastEdges = 0;               ///< 最近边数
        double avgProcessingTimeMs = 0.0;///< 平均耗时(ms)
        quint64 reachablePairs = 0;      ///< 可达顶点对数
    };

    explicit TransitiveClosure(QObject *parent = nullptr);
    ~TransitiveClosure() override;

    void setAlgorithm(Algorithm algo);

    /**
     * @brief 从邻接表构建并计算传递闭包
     * @param adjList 邻接表(adjList[u] = {v1, v2, ...})
     * @param n 顶点数
     */
    void build(const QVector<QVector<int>>& adjList, int n);

    /**
     * @brief 从边列表构建
     * @param edges 边列表(每条边为{from, to})
     * @param n 顶点数
     */
    void buildFromEdges(const QVector<QPair<int, int>>& edges, int n);

    /** @brief 查询u到v是否可达 */
    bool reachable(int u, int v) const;

    /** @brief 获取u可达的所有顶点 */
    QVector<int> reachableFrom(int u) const;

    /** @brief 获取完整闭包矩阵 */
    QVector<QVector<bool>> closureMatrix() const;

    /** @brief 获取可达顶点对数 */
    quint64 countReachablePairs() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 计算完成 @param vertices 顶点数 @param pairs 可达对数 */
    void closureComputed(int vertices, quint64 pairs);

private:
    /** @brief Floyd-Warshall算法 */
    void runFloydWarshall();

    /** @brief Warshall位集优化算法 */
    void runWarshallBitset();

    Algorithm m_algo = FloydWarshall;

    int m_n = 0;
    QVector<QVector<bool>> m_closure;   ///< 闭包矩阵

    Stats m_stats;
    double m_timeSum = 0.0;
};
