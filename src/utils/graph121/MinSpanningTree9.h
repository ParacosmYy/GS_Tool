#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 最小生成树(Minimum Spanning Tree)算法实现 (版本9)
 *
 * 基于Kruskal和Prim算法求解加权无向图的最小生成树，
 * 支持稀疏图优化和动态边权更新，适用于网络设计和聚类应用。
 */
class MinSpanningTree9 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalSolved = 0; double avgProcessingTimeMs = 0.0; };

    explicit MinSpanningTree9(QObject* parent = nullptr);

    /** @brief 设置图的节点数并清空边集 */
    void setNodeCount(int n);

    /** @brief 添加一条加权无向边(u, v, weight) */
    void addEdge(int u, int v, double weight);

    /** @brief 使用Kruskal算法计算最小生成树，返回选中的边集 */
    QVector<QPair<QPair<int, int>, double>> kruskal();

    /** @brief 使用Prim算法计算最小生成树，返回总权重 */
    double prim(int startNode = 0);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 最小生成树计算完成信号，返回边数和总权重 */
    void mstComputed(int edgeCount, double totalWeight);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_nodeCount = 0;
};
