#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 双连通分量(Biconnected Component)算法实现
 *
 * 基于Tarjan算法查找无向图中的双连通分量和割点(Articulation Points)，
 * 移除任意割点会导致图不连通，适用于网络可靠性分析和故障容忍设计。
 */
class Biconnected7 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalSolved = 0; double avgProcessingTimeMs = 0.0; };

    explicit Biconnected7(QObject* parent = nullptr);

    /** @brief 设置图的节点数并清空边集 */
    void setNodeCount(int n);

    /** @brief 添加一条无向边(u, v) */
    void addEdge(int u, int v);

    /** @brief 查找所有双连通分量，每个分量返回边列表 */
    QVector<QVector<QPair<int, int>>> findBiconnectedComponents();

    /** @brief 查找所有割点(删除后图不连通的节点) */
    QVector<int> findArticulationPoints();

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 双连通分量查找完成信号，返回分量数和割点数 */
    void componentsFound(int componentCount, int articulationPointCount);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_nodeCount = 0;
};
