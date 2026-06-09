/**
 * @file DominatingSet9.h
 * @brief 支配集(连通变体生成树强制+最小Steiner顶点增广) — Dominating Set with Connected Variant via Spanning Tree Enforcement and Minimal Steiner Vertex Augmentation
 *
 * 功能: 实现最小支配集(Minimum Dominating Set)，支持连通变体通过生成树
 *       强制(spanning tree enforcement)确保连通性，使用最小Steiner顶点
 *       增广(minimal Steiner vertex augmentation)连接孤立支配顶点。
 *
 * 协作: GraphColoring9(图着色) / MaxFlow9(最大流) / ShortestPath9(最短路径)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 支配集(连通变体+Steiner增广)
 */
class DominatingSet9 : public QObject {
    Q_OBJECT

public:
    /** @brief Result of dominating set computation */
    struct DomResult {
        QVector<int> dominatingSet;
        bool isConnected = false;
        int numSteinerAdded = 0;
        double dominationRatio = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numVertices = 0;
        int numEdges = 0;
        int dominatingSetSize = 0;
        int numSteinerVertices = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit DominatingSet9(QObject *parent = nullptr);
    ~DominatingSet9() override;

    /** @brief Enable connected dominating set variant */
    void setConnected(bool connected);

    /** @brief Compute dominating set from adjacency list */
    DomResult compute(const QVector<QVector<int>>& adjacency);

    /** @brief Verify if a set is a valid dominating set */
    bool verifyDomination(const QVector<QVector<int>>& adjacency,
                           const QVector<int>& candidate) const;

    /** @brief Compute Steiner vertices to connect a set */
    QVector<int> steinerAugment(const QVector<QVector<int>>& adjacency,
                                 const QVector<int>& terminals) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void computationCompleted(int setSize, int steinerAdded, double timeMs);

private:
    bool m_connected = false;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Greedy dominating set selection */
    QVector<int> greedyDominatingSet(const QVector<QVector<int>>& adj) const;

    /** @brief BFS shortest path between two vertices */
    QVector<int> bfsPath(const QVector<QVector<int>>& adj,
                          int src, int dst) const;

    /** @brief Check if dominating set induces a connected subgraph */
    bool isConnected(const QVector<QVector<int>>& adj,
                      const QVector<int>& vertices) const;

    /** @brief Enforce connectivity via spanning tree */
    QVector<int> enforceSpanningTree(const QVector<QVector<int>>& adj,
                                      QVector<int>& domSet) const;

    /** @brief Count undominated neighbors for greedy heuristic */
    int countUndominated(const QVector<QVector<int>>& adj,
                          const QVector<bool>& dominated, int v) const;
};
