/**
 * @file Biconnected10.h
 * @brief 双连通分量(2-边连通分解+桥块树构建) — Biconnected Components with 2-Edge-Connected Decomposition and Bridge-Block Tree Construction
 *
 * 功能: 实现双连通分量算法，支持割点/桥检测、
 *       2-边连通分量分解和桥块树(Bridge-Block Tree)构建。
 *
 * 协作: TarjanSCC8(Tarjan强连通) / EulerTour7(欧拉回路) / MinSpanTree7(最小生成树)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 双连通分量(桥检测+桥块树)
 */
class Biconnected10 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalOperations = 0;
        int numVertices = 0;
        int numEdges = 0;
        int numBridges = 0;
        int numComponents = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Biconnected10(QObject *parent = nullptr);
    ~Biconnected10() override;

    /** @brief Build graph with n vertices */
    void initGraph(int n);

    /** @brief Add undirected edge (u, v) */
    void addEdge(int u, int v);

    /** @brief Find all bridges and biconnected components via DFS */
    void compute();

    /** @brief Get list of bridges (u, v) */
    QVector<QPair<int, int>> bridges() const;

    /** @brief Get articulation points (cut vertices) */
    QVector<int> articulationPoints() const;

    /** @brief Get biconnected components (each is a list of vertices) */
    QVector<QVector<int>> components() const;

    /** @brief Build bridge-block tree: returns adjacency list of the block tree */
    QVector<QVector<int>> bridgeBlockTree() const;

    /** @brief Get vertex-to-component mapping */
    QVector<int> vertexComponent() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void computationCompleted(int bridges, int components, double timeMs);

private:
    int m_n = 0;
    QVector<QVector<int>> m_adj;

    // DFS state
    QVector<int> m_disc;
    QVector<int> m_low;
    QVector<int> m_parent;
    QVector<bool> m_visited;

    QVector<QPair<int, int>> m_bridges;
    QVector<int> m_articulations;
    QVector<QVector<int>> m_components;
    QVector<int> m_vertComp;

    Stats m_stats;
    double m_timeSum = 0.0;
    int m_timer = 0;

    /** @brief DFS for bridge and articulation point detection */
    void dfs(int u);

    /** @brief DFS for biconnected component extraction using edge stack */
    void dfsBcc(int u, QVector<QPair<int, int>>& edgeStack);
};
