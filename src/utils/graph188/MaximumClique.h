/**
 * @file MaximumClique.h
 * @brief 最大团Bron-Kerbosch算法(带枢轴退化序) — Maximum Clique via Bron-Kerbosch with Pivot Selection and Degeneracy Ordering
 *
 * 功能: 实现最大团检测，支持Bron-Kerbosch枚举(带枢轴剪枝)、
 *       退化序(Core Decomposition)预处理加速。
 *
 * 协作: GraphColoring6(图着色) / ShortestPath8(最短路径) / MinSpanTree8(最小生成树)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 最大团求解器
 */
class MaximumClique : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalSearches = 0;       ///< 累计搜索次数
        int lastCliqueSize = 0;          ///< 最近找到的团大小
        int maxCliqueSize = 0;           ///< 历史最大团大小
        quint64 nodesExplored = 0;       ///< 最近搜索的节点数
        double avgProcessingTimeMs = 0.0;///< 平均耗时(ms)
    };

    explicit MaximumClique(QObject *parent = nullptr);
    ~MaximumClique() override;

    /**
     * @brief 从邻接矩阵建图
     * @param adj 邻接矩阵(adj[i][j]=1表示有边)
     */
    void buildFromMatrix(const QVector<QVector<int>>& adj);

    /**
     * @brief 从边列表建图
     * @param edges 边列表 [{u,v}, ...]
     * @param n 节点数
     */
    void buildFromEdges(const QVector<QPair<int, int>>& edges, int n);

    /** @brief 求最大团 */
    QVector<int> findMaximumClique();

    /** @brief 枚举所有极大团 */
    QVector<QVector<int>> enumerateAllMaximal();

    /** @brief 退化序预处理 */
    QVector<int> degeneracyOrdering() const;

    /** @brief 求最大团大小(不返回具体节点) */
    int maxCliqueSize();

    int nodeCount() const;
    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void searchCompleted(int cliqueSize, quint64 nodesExplored);
    void cliqueFound(int size);

private:
    /** @brief Bron-Kerbosch with pivot (Tomita variant) */
    void bronKerboschPivot(QVector<int>& R, QVector<int>& P, QVector<int>& X);

    /** @brief 枚举所有极大团 */
    void bronKerboschEnumerate(QVector<int>& R, QVector<int>& P, QVector<int>& X,
                                QVector<QVector<int>>& results);

    /** @brief 计算节点度数 */
    QVector<int> computeDegrees() const;

    /** @brief 计算上界(着色)用于剪枝 */
    int greedyColoring(const QVector<int>& vertices) const;

    int m_n = 0;                              ///< 节点数
    QVector<QVector<int>> m_adj;              ///< 邻接表
    QVector<int> m_bestClique;                ///< 当前最大团
    quint64 m_nodesExplored = 0;              ///< 当前搜索节点数

    Stats m_stats;
    double m_timeSum = 0.0;
};
