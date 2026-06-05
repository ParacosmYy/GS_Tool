/**
 * @file MaxClique.h
 * @brief 最大团搜索 — Bron-Kerbosch回溯算法
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QList>
#include <QSet>

/**
 * @brief 最大团搜索引擎
 * Bron-Kerbosch带枢轴剪枝,支持最大团枚举和k-团计数
 */
class MaxClique : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalSearches = 0;            ///< 累计搜索次数
        int totalCliquesFound = 0;        ///< 累计找到团数
        int totalNodesExplored = 0;       ///< 累计探索节点数
        double avgProcessingTimeMs = 0.0;
    };

    explicit MaxClique(QObject* parent = nullptr);

    /** @brief 设置图邻接表 @param adj 邻接表(adj[u]=邻居列表) @param n 节点数 */
    void setGraph(const QVector<QVector<int>>& adj, int n);

    /** @brief 找最大团 @return 最大团的节点列表 */
    QVector<int> findMaximum();

    /** @brief 找所有极大团 @param maxSize 最大返回团数(0=全部) @return 团列表 */
    QList<QVector<int>> findAllMaximal(int maxSize = 0);

    /** @brief 判断是否为团 @param nodes 节点集 */
    bool isClique(const QVector<int>& nodes) const;

    /** @brief 统计k-团数量 @param k 团大小 @return 数量 */
    long long countKCliques(int k) const;

    /** @brief 获取图染色数上界(团数下界) */
    int chromaticLowerBound() const;

    /** @brief 获取节点数 */
    int nodeCount() const { return m_n; }

    /** @brief 获取最大团大小 */
    int maximumSize() const { return m_maxSize; }

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 搜索完成 @param cliqueSize 最大团大小 @param explored 探索节点数 */
    void searchCompleted(int cliqueSize, int explored);

private:
    /** @brief Bron-Kerbosch递归(带枢轴) */
    void bronKerboschPivot(QVector<int>& R, QVector<int>& P, QVector<int>& X);

    /** @brief 枚举所有极大团 */
    void enumerateAll(QVector<int>& R, QVector<int>& P, QVector<int>& X,
                      QList<QVector<int>>& result, int maxSize);

    int m_n = 0;                          ///< 节点数
    QVector<QVector<int>> m_adj;          ///< 邻接表
    QVector<QSet<int>> m_adjSet;          ///< 邻接集合(快速查询)
    QVector<int> m_bestClique;            ///< 当前最大团
    int m_maxSize = 0;                    ///< 最大团大小

    Stats m_stats;
    double m_timeSum = 0.0;
};
