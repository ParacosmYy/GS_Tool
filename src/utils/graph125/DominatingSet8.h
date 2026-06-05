#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 支配集(Dominating Set)算法实现
 *
 * 查找图中满足条件的顶点子集：每个顶点要么在集合中，要么与集合中某顶点相邻，
 * 支持贪心近似和精确求解，适用于无线传感器网络部署和设施选址问题。
 */
class DominatingSet8 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalSolved = 0; double avgProcessingTimeMs = 0.0; };

    explicit DominatingSet8(QObject* parent = nullptr);

    /** @brief 设置图的节点数并清空边集 */
    void setNodeCount(int n);

    /** @brief 添加一条无向边(u, v) */
    void addEdge(int u, int v);

    /** @brief 使用贪心近似算法计算最小支配集，返回顶点索引列表 */
    QVector<int> greedyApprox();

    /** @brief 检查给定顶点集合是否构成有效支配集 */
    bool isDominatingSet(const QVector<int>& candidates) const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 支配集计算完成信号，返回集合大小 */
    void setComputed(int setSize);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_nodeCount = 0;
};
