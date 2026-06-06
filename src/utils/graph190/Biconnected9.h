/**
 * @file Biconnected9.h
 * @brief 双连通分量分解(Tarjan DFS关节点检测) — Biconnected Component Decomposition via Tarjan DFS with Articulation Point Detection
 *
 * 功能: 实现双连通分量分解算法，支持Tarjan DFS检测关节点(割点)、
 *       桥检测和双连通分量枚举。
 *
 * 协作: StronglyConnected9(强连通) / TopologicalSort8(拓扑排序) / CycleDetect6(环检测)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 双连通分量分解器(Tarjan DFS)
 */
class Biconnected9 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalRuns = 0;            ///< 累计运行次数
        int numComponents = 0;            ///< 双连通分量数
        int numArticulations = 0;         ///< 关节点数
        int numBridges = 0;               ///< 桥数
        int numVertices = 0;              ///< 顶点数
        int numEdges = 0;                 ///< 边数
        double avgProcessingTimeMs = 0.0; ///< 平均耗时(ms)
    };

    explicit Biconnected9(QObject *parent = nullptr);
    ~Biconnected9() override;

    /**
     * @brief 执行双连通分量分解
     * @param adj 邻接表(顶点->相邻顶点列表)
     * @return 每条边所属的双连通分量编号(-1表示桥)
     */
    QVector<int> decompose(const QVector<QVector<int>>& adj);

    /** @brief 获取关节点列表 */
    QVector<int> articulationPoints() const;

    /** @brief 获取桥列表 {u, v} */
    QVector<QPair<int, int>> bridges() const;

    /** @brief 获取各双连通分量的边集 */
    QVector<QVector<QPair<int, int>>> components() const;

    /** @brief 检查图是否双连通 */
    bool isBiconnected(const QVector<QVector<int>>& adj) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void decompositionCompleted(int numComponents, int numArticulations);
    void articulationFound(int vertex);
    void bridgeFound(int u, int v);

private:
    /** @brief Tarjan DFS递归 */
    void dfs(int u, int parent, const QVector<QVector<int>>& adj,
             QVector<int>& disc, QVector<int>& low,
             QVector<bool>& visited, int& timer);

    /** @brief 使用栈追踪双连通分量 */
    void extractComponent(int u, int v);

    QVector<int> m_articulations;               ///< 关节点
    QVector<QPair<int, int>> m_bridges;         ///< 桥
    QVector<QVector<QPair<int, int>>> m_comps;  ///< 双连通分量

    /* DFS tracking */
    QVector<QPair<int, int>> m_edgeStack;
    QVector<int> m_edgeLabels;

    Stats m_stats;
    double m_timeSum = 0.0;
};
