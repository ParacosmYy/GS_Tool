/**
 * @file ArticulationPoint.h
 * @brief 关节点与桥检测器
 */

#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 关节点与桥检测器
 *
 * 使用Tarjan算法在O(V+E)时间内找出无向图中的
 * 所有关节点(割点)和桥边。
 */
class ArticulationPoint : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalSearches = 0;             ///< 总搜索次数
        int totalArticulations = 0;        ///< 总关节点数
        int totalBridges = 0;              ///< 总桥数
        double avgProcessingTimeMs = 0.0;
    };

    explicit ArticulationPoint(QObject* parent = nullptr);

    /**
     * @brief 添加无向边
     */
    void addEdge(int u, int v);

    /**
     * @brief 查找所有关节点
     * @return 关节点集合
     */
    QVector<int> findArticulationPoints();

    /**
     * @brief 查找所有桥
     * @return 桥边列表{u,v}
     */
    QVector<QPair<int, int>> findBridges();

    /**
     * @brief 同时查找关节点和桥
     */
    void findAll();

    /**
     * @brief 获取关节点列表(调用findAll后)
     */
    QVector<int> articulationPoints() const;

    /**
     * @brief 获取桥列表(调用findAll后)
     */
    QVector<QPair<int, int>> bridges() const;

    /**
     * @brief 重置图
     */
    void reset();

    Stats stats() const;
    void resetStatistics();

signals:
    /** @brief 搜索完成信号 */
    void searchCompleted(int articulationCount, int bridgeCount);

private:
    QVector<QVector<int>> m_adj;  ///< 邻接表
    QVector<int> m_disc;          ///< 发现时间
    QVector<int> m_low;           ///< 最低可达时间
    QVector<bool> m_visited;
    QVector<bool> m_isArticulation;
    QVector<QPair<int, int>> m_bridges;
    int m_time;

    Stats m_stats;
    double m_timeSum = 0.0;

    void dfs(int u, int parent);
    void ensureGraph(int n);
};
