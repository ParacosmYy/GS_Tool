#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 强连通分量算法实现 (版本5)
 *
 * 基于Tarjan算法求解有向图的强连通分量，支持拓扑排序和缩图构建。
 */
class StronglyConnected5 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构
    struct Stats {
        int totalSearches = 0;          ///< 总搜索次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
        int componentCount = 0;         ///< 最近一次的分量数
    };

    explicit StronglyConnected5(QObject* parent = nullptr);
    Stats stats() const { return m_stats; }
    void resetStatistics();

    /**
     * @brief Tarjan算法求强连通分量
     * @param adjacencyList 邻接表表示的有向图
     * @return 每个顶点的分量编号
     */
    QVector<int> tarjan(const QVector<QVector<int>>& adjacencyList);

    /**
     * @brief Kosaraju算法求强连通分量
     * @param adjacencyList 邻接表
     * @return 每个顶点的分量编号
     */
    QVector<int> kosaraju(const QVector<QVector<int>>& adjacencyList);

    /**
     * @brief 构建缩图（每个SCC缩为一个节点）
     * @param adjacencyList 原图邻接表
     * @param componentIds 分量编号
     * @return 缩图的邻接表
     */
    QVector<QVector<int>> condensationGraph(const QVector<QVector<int>>& adjacencyList,
                                             const QVector<int>& componentIds) const;

    /**
     * @brief 检查图是否为强连通图
     * @param adjacencyList 邻接表
     * @return 是否强连通
     */
    bool isStronglyConnected(const QVector<QVector<int>>& adjacencyList) const;

signals:
    /// 搜索完成信号
    void searchCompleted(int componentCount);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_componentCount = 0;
};
