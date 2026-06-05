/**
 * @file PlanarCheck.h
 * @brief 平面图判定 — Hopcroft-Tarjan平面性测试+Kuratowski子图提取
 *
 * 功能: Hopcroft-Tarjan平面性测试算法，Kuratowski子图(K5/K3,3)提取，
 *       st-编号(双连通分量)，边嵌入信息记录，判定图是否可平面嵌入。
 *
 * 协作: StateTracker(状态图分析) / DataCorrelator(依赖图)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>

/**
 * @brief 平面图检测引擎
 *
 * 使用 Hopcroft-Tarjan DFS 算法判定图是否为平面图，
 * 若非平面图则尝试提取 Kuratowski 子图(K5 或 K3,3)。
 */
class PlanarCheck : public QObject {
    Q_OBJECT

public:
    /** @brief Kuratowski子图类型 */
    enum class KuratowskiType {
        None,       ///< 无(图是平面图)
        K5,         ///< 完全图K5
        K33         ///< 完全二分图K3,3
    };
    Q_ENUM(KuratowskiType)

    /** @brief 检测结果 */
    struct PlanarResult {
        bool isPlanar = true;                   ///< 是否平面图
        KuratowskiType kuratowskiType = KuratowskiType::None; ///< Kuratowski类型
        QList<QPair<int, int>> kuratowskiEdges; ///< Kuratowski子图的边
        int vertexCount = 0;                    ///< 顶点数
        int edgeCount = 0;                      ///< 边数
    };

    /** @brief 统计信息 */
    struct Stats {
        quint64 totalChecks = 0;                ///< 累计检测次数
        quint64 totalPlanarGraphs = 0;          ///< 累计平面图数
        quint64 totalNonPlanarGraphs = 0;       ///< 累计非平面图数
        double  avgProcessingTimeMs = 0.0;      ///< 平均处理时间(ms)
    };

    explicit PlanarCheck(QObject* parent = nullptr);

    PlanarResult check(int vertexCount, const QList<QPair<int, int>>& edges);
    PlanarResult checkAdjacency(const QVector<QVector<int>>& adjMatrix);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void checkCompleted(bool isPlanar, int vertices, int edges);

private:
    void buildGraph(int n, const QList<QPair<int, int>>& edges);
    bool hopcroftTarjan();
    void dfs(int v, int parent);
    bool checkInterleaving(int v, int w);
    void extractKuratowski();
    void findK33();
    void findK5();
    QList<int> findCycle(int start, int end) const;

    /* 图结构 */
    int m_n;                                        ///< 顶点数
    QVector<QVector<int>> m_adj;                    ///< 邻接表
    QList<QPair<int, int>> m_edges;                 ///< 边列表

    /* DFS状态 */
    QVector<int> m_dfsNum;                          ///< DFS编号
    QVector<int> m_parent;                          ///< 父节点
    QVector<int> m_lowPt;                           ///< 最低可达点
    QVector<int> m_lowPt2;                          ///< 次低可达点
    int m_dfsCounter;                               ///< DFS计数器

    /* 结果缓存 */
    PlanarResult m_result;                          ///< 检测结果

    Stats m_stats;
    double m_timeSum = 0.0;
};
