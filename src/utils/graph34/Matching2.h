/**
 * @file Matching2.h
 * @brief 一般图匹配引擎 — Edmonds花算法/最大基数匹配/交错路径/花收缩
 *
 * 功能: 实现Edmonds开花算法(Blossom Algorithm)，在一般图中
 *       求解最大基数匹配，支持花(blossom)检测与收缩、
 *       交错路径搜索、匹配增广等核心操作。
 *
 * 协作: DataCorrelator(数据关联) / CycleDetector(环检测)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>

/**
 * @brief 一般图匹配引擎 — Edmonds花算法与最大基数匹配
 */
class Matching2 : public QObject {
    Q_OBJECT

public:
    /** @brief 匹配结果 */
    struct MatchResult {
        QVector<QPair<int, int>> edges;     ///< 匹配边列表
        int cardinality = 0;                ///< 匹配基数
        bool isPerfect = false;             ///< 是否完美匹配
        int blossomCount = 0;              ///< 收缩的花数量
    };

    /** @brief 统计 */
    struct Stats {
        quint64 totalMatches = 0;               ///< 累计匹配计算次数
        quint64 totalVerticesProcessed = 0;     ///< 累计处理顶点数
        quint64 totalEdgesProcessed = 0;        ///< 累计处理边数
        quint64 totalBlossomsContracted = 0;    ///< 累计收缩花数量
        double  avgProcessingTimeMs = 0.0;      ///< 平均处理时间(ms)
    };

    explicit Matching2(QObject* parent = nullptr);

    /** @brief 从邻接表构建图 @param adjList 邻接表 @param numVertices 顶点数 */
    void buildGraph(const QVector<QVector<int>>& adjList, int numVertices);

    /** @brief 从边列表构建图 @param edges 边列表 @param numVertices 顶点数 */
    void buildFromEdges(const QVector<QPair<int, int>>& edges, int numVertices);

    /** @brief 计算最大基数匹配 @return 匹配结果 */
    MatchResult maximumMatching();

    /** @brief 检查边是否在匹配中 @param u 顶点u @param v 顶点v @return 是否匹配边 */
    bool isMatched(int u, int v) const;

    /** @brief 获取顶点的匹配伙伴 @param v 顶点 @return 匹配伙伴(-1表示未匹配) */
    int matchOf(int v) const;

    /** @brief 获取顶点数 @return 顶点数 */
    int vertexCount() const { return m_numVertices; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 匹配完成 @param cardinality 匹配基数 @param blossoms 花数量 */
    void matchingComplete(int cardinality, int blossoms);

private:
    int lca(int a, int b);
    void tracePath(QVector<int>& path, int v, int ancestor);
    void blossomContract(int u, int v, int lcaVertex);
    bool augment();
    void bfs(int root);

    int m_numVertices;                  ///< 顶点数
    QVector<QVector<int>> m_adj;        ///< 邻接表
    QVector<int> m_match;               ///< 匹配数组
    QVector<int> m_parent;              ///< BFS父节点
    QVector<int> m_base;                ///< 花的基顶点
    QVector<bool> m_inBlossom;          ///< 是否在花中
    QVector<bool> m_visited;            ///< BFS访问标记
    int m_blossomCount;                 ///< 当前收缩花数量

    Stats m_stats;
    double m_timeSum = 0.0;             ///< 处理时间累加器
};
