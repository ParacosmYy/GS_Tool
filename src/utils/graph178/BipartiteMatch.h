/**
 * @file BipartiteMatch.h
 * @brief 二分图最大匹配(Hopcroft-Karp) — Hopcroft-Karp Maximum Bipartite Matching with BFS Layering
 *
 * 功能: 实现Hopcroft-Karp算法求解二分图最大匹配。
 *       使用BFS分层和DFS增广，O(E*sqrt(V))复杂度。
 *       支持最小顶点覆盖和最大独立集计算。
 *
 * 协作: MaxFlow7(最大流) / GraphColoring(图着色) / TopologicalSort(拓扑排序)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief Hopcroft-Karp二分图最大匹配
 */
class BipartiteMatch : public QObject {
    Q_OBJECT

public:
    /** @brief 匹配结果 */
    struct MatchResult {
        QVector<QPair<int, int>> matches;  ///< 匹配边(左->右)
        int matchCount;                     ///< 匹配数
        QVector<int> leftMatch;            ///< 左侧顶点的匹配(-1=未匹配)
        QVector<int> rightMatch;           ///< 右侧顶点的匹配(-1=未匹配)
    };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalMatches = 0;          ///< 累计匹配次数
        quint64 totalBfsPhases = 0;        ///< 累计BFS层数
        quint64 totalDfsPaths = 0;         ///< 累计DFS增广路径
        double avgProcessingTimeMs = 0.0;  ///< 平均处理耗时(ms)
    };

    explicit BipartiteMatch(QObject* parent = nullptr);
    ~BipartiteMatch() override;

    /**
     * @brief 设置二分图
     * @param leftSize 左侧顶点数
     * @param rightSize 右侧顶点数
     * @param edges 边列表(left, right)
     */
    void setGraph(int leftSize, int rightSize, const QVector<QPair<int, int>>& edges);

    /**
     * @brief 添加单条边
     * @param left 左侧顶点
     * @param right 右侧顶点
     */
    void addEdge(int left, int right);

    /** @brief 清空图 */
    void clearGraph();

    /**
     * @brief 运行Hopcroft-Karp最大匹配
     * @return 匹配结果
     */
    MatchResult findMaxMatching();

    /**
     * @brief 从匹配结果计算最小顶点覆盖(Konig定理)
     * @param result 匹配结果
     * @return 顶点覆盖中的顶点列表
     */
    QVector<int> minVertexCover(const MatchResult& result) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 匹配完成 @param count 匹配数 */
    void matchingCompleted(int count);

private:
    /** @brief BFS分层：构建距离标签 */
    bool bfsLayering(QVector<int>& dist);

    /** @brief DFS增广：沿分层图寻找增广路 */
    bool dfsAugment(int u, const QVector<int>& dist, QVector<bool>& visited);

    int m_leftSize = 0;
    int m_rightSize = 0;
    QVector<QVector<int>> m_adj;    ///< 邻接表: left -> [right vertices]
    QVector<int> m_leftMatch;       ///< 当前匹配: left->right
    QVector<int> m_rightMatch;      ///< 当前匹配: right->left

    Stats m_stats;
    double m_timeSum = 0.0;
};
