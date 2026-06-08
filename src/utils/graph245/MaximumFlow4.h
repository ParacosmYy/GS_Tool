/**
 * @file MaximumFlow4.h
 * @brief 最大流(Highest-Label推重标签+间隙启发式O(V^2√E)) — Maximum Flow with Push-Relabel Highest-Label Selection and Gap Heuristic for O(V^2√E) Performance
 *
 * 功能: 实现最大流(maximum flow)算法，采用推重标签(push-relabel)方法，
 *       使用highest-label选择策略和间隙启发式(gap heuristic)达到O(V^2√E)性能。
 *
 * 协作: MinimumCostFlow3(最小费用流) / Dijkstra7(最短路) / BipartiteMatcher4(二分图匹配)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 最大流(Highest-Label推重标签+间隙启发式)
 */
class MaximumFlow4 : public QObject {
    Q_OBJECT

public:
    /** @brief Edge descriptor */
    struct Edge {
        int to = -1;
        int rev = -1;       // reverse edge index
        double capacity = 0.0;
        double flow = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numVertices = 0;
        int numEdges = 0;
        double maxFlow = 0.0;
        int numPushes = 0;
        int numRelabels = 0;
        int numGaps = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit MaximumFlow4(QObject *parent = nullptr);
    ~MaximumFlow4() override;

    /** @brief Initialize graph with n vertices */
    void initGraph(int n);

    /** @brief Add directed edge with capacity (and optional reverse capacity) */
    void addEdge(int from, int to, double capacity, double revCapacity = 0.0);

    /** @brief Compute maximum flow from source to sink */
    double maxFlow(int source, int sink);

    /** @brief Get min-cut partition (vertices reachable from source in residual) */
    QVector<int> minCut(int source) const;

    /** @brief Get flow on a specific edge */
    double edgeFlow(int from, int edgeIdx) const;

    /** @brief Get all edges from a vertex */
    QVector<Edge> edgesFrom(int vertex) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void flowComputed(double maxFlow, int pushes, int relabels, double timeMs);
    void gapDetected(int gapLevel, int verticesAffected);

private:
    int m_n = 0;
    QVector<QVector<Edge>> m_graph;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Push flow from vertex u along an admissible edge */
    bool push(int u, QVector<double>& excess, QVector<int>& height);

    /** @brief Relabel vertex u to minimum neighbor height + 1 */
    void relabel(int u, const QVector<double>& excess, QVector<int>& height);

    /** @brief Gap heuristic: relabel all vertices above gap level */
    void gapHeuristic(int gapLevel, QVector<int>& height, QVector<double>& excess);

    /** @brief Discharge a vertex until excess is zero */
    void discharge(int u, QVector<double>& excess, QVector<int>& height,
                   QVector<int>& currentEdge, int sink);

    /** @brief BFS to find reachable vertices in residual graph */
    QVector<int> bfsReachable(int source, const QVector<int>& height) const;
};
