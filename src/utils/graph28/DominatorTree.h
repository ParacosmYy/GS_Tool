/**
 * @file DominatorTree.h
 * @brief 支配树构造 — Lengauer-Tarjan算法 + 直接支配者 + 支配边界
 *
 * 功能: 对有向图的节点构造支配树，计算每个节点的直接支配者(idom)和支配边界。
 *       使用Lengauer-Tarjan算法(近线性复杂度)，支持DFS编号、半支配者计算。
 *       统计构造次数/节点数/平均处理耗时。
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QMap>
#include <QPair>

/**
 * @class DominatorTree
 * @brief 基于Lengauer-Tarjan算法的支配树构造器
 */
class DominatorTree : public QObject {
    Q_OBJECT
public:
    /** 图的边表示 */
    struct Edge {
        int from; ///< 起始节点
        int to;   ///< 目标节点
    };

    /** 节点支配信息 */
    struct DomInfo {
        int idom = -1;                  ///< 直接支配者(-1表示无)
        QVector<int> dominatedBy;       ///< 被支配的节点列表
        QVector<int> dominanceFrontier; ///< 支配边界
    };

    /** 构造统计 */
    struct Stats {
        quint64 totalConstructions = 0;  ///< 总构造次数
        quint64 totalNodesProcessed = 0; ///< 累计处理节点数
        quint64 totalEdgesProcessed = 0; ///< 累计处理边数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    /** 构造函数 */
    explicit DominatorTree(QObject* parent = nullptr);

    /**
     * @brief 从边列表构造支配树
     * @param edges 有向边列表
     * @param numNodes 节点总数
     * @param entryNode 入口节点编号
     * @return 每个节点的支配信息
     */
    QVector<DomInfo> build(const QVector<Edge>& edges, int numNodes, int entryNode = 0);

    /**
     * @brief 计算支配边界
     * @param domInfo 支配信息(从build获取)
     * @param numNodes 节点总数
     * @return 更新后的支配信息(含支配边界)
     */
    QVector<DomInfo> computeDominanceFrontiers(
        const QVector<DomInfo>& domInfo, int numNodes) const;

    /**
     * @brief 检查节点a是否支配节点b
     * @param domInfo 支配信息
     * @param a 支配节点
     * @param b 被支配节点
     * @return true表示a支配b
     */
    bool dominates(const QVector<DomInfo>& domInfo, int a, int b) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 支配树构造完成 @param nodes 节点数 @param edges 边数 */
    void constructionComplete(int nodes, int edges);
    /** @brief 支配边界计算完成 @param frontierSize 总边界大小 */
    void frontiersComputed(int frontierSize);

private:
    /** DFS遍历编号 */
    void dfs(int v, const QVector<QVector<int>>& succ,
             QVector<int>& semi, QVector<int>& vertex,
             QVector<int>& parent, int& counter);
    /** 并查集Find(路径压缩) */
    int find(int v, QVector<int>& ancestor, QVector<int>& label,
             const QVector<int>& semi);

    mutable Stats  m_stats;         ///< 统计信息
    mutable double m_timeSum = 0.0; ///< 累计耗时
};
