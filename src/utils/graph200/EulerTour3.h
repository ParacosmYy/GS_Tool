/**
 * @file EulerTour3.h
 * @brief 欧拉回路(Hierholzer+Fleury桥避让变体) — Euler Tour via Hierholzer's Algorithm with Fleury's Bridge-Avoidance Variant
 *
 * 功能: 实现欧拉回路/欧拉路径，支持Hierholzer算法、
 *       Fleury桥避让变体、桥检测(Tarjan)和回路验证。
 *
 * 协作: HamiltonPath4(哈密顿路径) / StrongConnect6(强连通) / TopoSort5(拓扑排序)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 欧拉回路求解器(Hierholzer+Fleury变体)
 */
class EulerTour3 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalRuns = 0;
        int numVertices = 0;
        int numEdges = 0;
        bool hasEulerTour = false;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief 欧拉回路结果 */
    struct TourResult {
        QVector<int> vertices;    // Vertex sequence of the tour
        QVector<int> edges;       // Edge index sequence
        bool isEulerTour = false;
        bool isEulerPath = false;
    };

    explicit EulerTour3(QObject *parent = nullptr);
    ~EulerTour3() override;

    /** @brief 添加无向边, 返回边编号 */
    int addEdge(int u, int v);

    /** @brief 设置顶点数 */
    void setNumVertices(int n);

    /** @brief Hierholzer算法求欧拉回路 */
    TourResult hierholzer() const;

    /** @brief Fleury桥避让变体求欧拉回路 */
    TourResult fleury() const;

    /** @brief 检测图是否含欧拉回路 */
    bool hasEulerCircuit() const;

    /** @brief 检测图是否含欧拉路径 */
    bool hasEulerPath() const;

    /** @brief Tarjan桥检测 */
    QVector<QPair<int, int>> findBridges() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void tourFound(int vertexCount, int edgeCount);

private:
    int m_numVertices = 0;
    QVector<QVector<QPair<int, int>>> m_adj; // adj[u] = {(v, edgeIdx)}

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Tarjan bridge DFS */
    void bridgeDfs(int u, int parent, QVector<bool>& visited,
                   QVector<int>& disc, QVector<int>& low,
                   int& time, QVector<QPair<int, int>>& bridges) const;

    /** @brief Count degree of vertex */
    int degree(int v) const;

    /** @brief Count odd-degree vertices */
    int oddDegreeCount() const;

    /** @brief Find Eulerian start vertex */
    int findStart(bool circuit) const;
};
