#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief 支配集(Dominating Set)求解器
 *
 * 寻找图中使每个顶点要么属于集合要么与集合中顶点相邻的最小顶点集,
 * 适用于无线网络节点部署、社交网络影响力分析与设施选址问题。
 */
class DominatingSet6 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalSolved = 0; double avgProcessingTimeMs = 0.0; };

    explicit DominatingSet6(QObject* parent = nullptr);

    /** @brief 设置顶点数量 */
    void setVertexCount(int count);

    /** @brief 添加无向边 */
    void addEdge(int from, int to);

    /** @brief 求解最小支配集 */
    void solve();

    /** @brief 获取支配集大小 */
    void coverSize();

    /** @brief 获取当前统计信息 */
    Stats stats() const { return m_stats; }

    /** @brief 重置统计数据 */
    void resetStatistics();

signals:
    /** @brief 求解完成信号,返回支配集大小 */
    void solved(int setSize);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_vertexCount = 0;
};
