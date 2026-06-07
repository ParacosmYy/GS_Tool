/**
 * @file Biconnected11.h
 * @brief 双连通分量分解(Hopcroft-Tarjan割点隔离) — Biconnected Component Decomposition via Hopcroft-Tarjan with Articulation Point Isolation
 *
 * 功能: 实现双连通分量分解，支持Hopcroft-Tarjan DFS、
 *       割点(关节点)识别与隔离、桥边检测。
 *
 * 协作: StronglyConnected7(强连通) / EulerPath5(欧拉路径) / TopologicalSort6(拓扑排序)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 双连通分量分解(Hopcroft-Tarjan割点隔离)
 */
class Biconnected11 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numVertices = 0;
        int numEdges = 0;
        int numComponents = 0;
        int numArticulations = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Biconnected11(QObject *parent = nullptr);
    ~Biconnected11() override;

    /** @brief Build adjacency list from edge list */
    void buildGraph(int n, const QVector<QPair<int, int>>& edges);

    /** @brief Find all biconnected components */
    QVector<QVector<int>> findBiconnectedComponents();

    /** @brief Find all articulation points */
    QVector<int> findArticulationPoints();

    /** @brief Find all bridge edges */
    QVector<QPair<int, int>> findBridges();

    /** @brief Check if removing vertex disconnects graph */
    bool isArticulation(int vertex) const;

    /** @brief Check if graph is biconnected */
    bool isBiconnected();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void decompositionCompleted(int components, int articulations, double timeMs);

private:
    int m_n = 0;
    QVector<QVector<int>> m_adj;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief DFS discovery time counter */
    int m_timer = 0;

    /** @brief Hopcroft-Tarjan DFS for articulation points */
    void articulationDFS(int u, int parent, QVector<int>& disc,
                          QVector<int>& low, QVector<bool>& visited,
                          QVector<bool>& isArt, QVector<int>& parentArr) const;

    /** @brief DFS for biconnected components using edge stack */
    void bccDFS(int u, int parent, QVector<int>& disc, QVector<int>& low,
                 QVector<bool>& visited, QVector<QPair<int, int>>& edgeStack,
                 QVector<QVector<int>>& components) const;

    /** @brief DFS for bridge detection */
    void bridgeDFS(int u, int parent, QVector<int>& disc, QVector<int>& low,
                    QVector<bool>& visited, QVector<QPair<int, int>>& bridges) const;
};
