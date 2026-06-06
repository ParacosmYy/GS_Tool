/**
 * @file GraphMatching2.h
 * @brief 一般图最大匹配(Edmonds花算法) — Maximum Matching in General Graphs via Edmonds' Blossom Algorithm
 *
 * 功能: 实现Edmonds花算法，支持一般图最大匹配、增广路搜索、
 *       花的检测与收缩、匹配恢复。
 *
 * 协作: HungarianAlgorithm3(匈牙利算法) / MaxFlow3(最大流) / BipartiteMatch2(二部匹配)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 一般图最大匹配器
 */
class GraphMatching2 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalMatches = 0;         ///< 累计匹配次数
        int lastMatchSize = 0;            ///< 最近匹配大小
        int lastVertices = 0;             ///< 最近顶点数
        int lastEdges = 0;                ///< 最近边数
        double avgProcessingTimeMs = 0.0; ///< 平均耗时(ms)
    };

    explicit GraphMatching2(QObject *parent = nullptr);
    ~GraphMatching2() override;

    /**
     * @brief 计算最大匹配
     * @param adjList 邻接表
     * @return 匹配边列表(u,v)，u<v
     */
    QVector<QPair<int, int>> maxMatching(const QVector<QVector<int>>& adjList);

    /** @brief 获取每个顶点的匹配顶点(-1未匹配) */
    QVector<int> matchVector() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void matchingCompleted(int matchSize, int vertices);

private:
    /** @brief BFS寻找增广路 */
    bool findAugmentingPath(int start, const QVector<QVector<int>>& adj);

    /** @brief 找公共祖先(LCA) */
    int findLCA(int u, int v, const QVector<QVector<int>>& adj);

    /** @brief 沿花边标记 */
    void markBlossomPath(int u, int ancestor, const QVector<QVector<int>>& adj);

    /** @brief 沿增广路翻转匹配 */
    void augmentPath(int end);

    QVector<int> m_match;     ///< match[v] = matched vertex, -1 if free
    QVector<int> m_parent;    ///< BFS parent
    QVector<int> m_base;      ///< Base of blossom
    QVector<bool> m_inBlossom;
    QVector<bool> m_visited;

    Stats m_stats;
    double m_timeSum = 0.0;
};
