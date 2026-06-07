/**
 * @file TransitiveClosure2.h
 * @brief 传递闭包(Floyd-Warshall+位打包稠密图优化) — Transitive Closure via Floyd-Warshall with Bit-Packing Optimization for Dense Graphs
 *
 * 功能: 实现传递闭包计算，支持Floyd-Warshall算法、位打包稠密图优化、
 *       Warshall传递闭包和可达性查询。
 *
 * 协作: StronglyConnected5(强连通分量) / ShortestPath7(最短路径) / TopologicalSort3(拓扑排序)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 传递闭包计算器(Floyd-Warshall+位打包优化)
 */
class TransitiveClosure2 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalRuns = 0;
        int numVertices = 0;
        int numEdges = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit TransitiveClosure2(QObject *parent = nullptr);
    ~TransitiveClosure2() override;

    /** @brief 设置邻接矩阵(0=无边, 1=有边) */
    void setAdjacencyMatrix(const QVector<QVector<int>>& matrix);

    /** @brief 设置边列表 */
    void setEdgeList(int numVertices, const QVector<QPair<int, int>>& edges);

    /** @brief 计算传递闭包(Floyd-Warshall) */
    QVector<QVector<int>> compute();

    /** @brief 计算传递闭包(位打包优化) */
    QVector<quint64> computeBitPacked();

    /** @brief 查询u是否可达v(需先计算闭包) */
    bool reachable(int u, int v) const;

    /** @brief 查询u是否可达v(位打包版) */
    bool reachablePacked(int u, int v) const;

    /** @brief 获取顶点u的所有可达顶点 */
    QVector<int> reachableSet(int u) const;

    /** @brief 计算强连通分量数(基于闭包) */
    int countStronglyConnected() const;

    int size() const { return m_numVertices; }
    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void closureComputed(int numVertices, double timeMs);

private:
    int m_numVertices = 0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Integer matrix closure */
    QVector<QVector<int>> m_closure;
    /** @brief Bit-packed closure (each quint64 holds 64 bits) */
    QVector<quint64> m_bitClosure;
    int m_bitWords = 0; // Number of quint64 per row

    /** @brief Ensure closure is computed */
    void ensureClosure();

    /** @brief Ensure bit-packed closure is computed */
    void ensureBitClosure();
};
