/**
 * @file StrongConnectivity3.h
 * @brief 强连通分量(Kosaraju两遍DFS+反向图) — Strongly Connected Components via Kosaraju's Two-Pass DFS with Reverse Graph
 *
 * 功能: 实现Kosaraju算法，支持两遍DFS强连通分量分解、
 *       反向图构建、拓扑排序和缩点DAG生成。
 *
 * 协作: TarjanSCC1(Tarjan) / TopologicalSort2(拓扑排序) / GraphBFS3(BFS图)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 强连通分量检测器(Kosaraju两遍DFS)
 */
class StrongConnectivity3 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalRuns = 0;
        int numVertices = 0;
        int numEdges = 0;
        int numComponents = 0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief SCC result */
    struct SCCResult {
        QVector<int> componentId;
        int numComponents = 0;
        QVector<QVector<int>> components;
        QVector<QPair<int, int>> dagEdges;
    };

    explicit StrongConnectivity3(QObject *parent = nullptr);
    ~StrongConnectivity3() override;

    /** @brief 从边列表构建图并计算SCC */
    SCCResult compute(int numVertices, const QVector<QPair<int, int>>& edges);

    /** @brief 构建反向图 */
    QVector<QVector<int>> buildReverseGraph(int n,
                                             const QVector<QVector<int>>& adj) const;

    /** @brief 从SCC结果构建缩点DAG */
    QVector<QPair<int, int>> buildCondensationDAG(
        const QVector<QPair<int, int>>& edges,
        const QVector<int>& componentId,
        int numComponents) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void computationCompleted(int numComponents, int largestSize);

private:
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief First DFS pass: compute finish order */
    void dfsOrder(int v, const QVector<QVector<int>>& adj,
                  QVector<bool>& visited, QVector<int>& order) const;

    /** @brief Second DFS pass: assign component IDs */
    void dfsAssign(int v, const QVector<QVector<int>>& revAdj,
                   QVector<bool>& visited, QVector<int>& compId, int id) const;
};
