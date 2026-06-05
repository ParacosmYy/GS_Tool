#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief 双连通分量与割点求解器
 *
 * 基于Tarjan算法识别无向图中的双连通分量(BCC)与割点(Articulation Point),
 * 适用于网络可靠性分析、社交网络关键节点发现与图结构优化。
 */
class Biconnected5 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalSolved = 0; double avgProcessingTimeMs = 0.0; };

    explicit Biconnected5(QObject* parent = nullptr);

    /** @brief 设置顶点数量 */
    void setVertexCount(int count);

    /** @brief 添加无向边 */
    void addEdge(int from, int to);

    /** @brief 求解双连通分量 */
    void solve();

    /** @brief 获取割点列表 */
    void articulationPoints();

    /** @brief 获取当前统计信息 */
    Stats stats() const { return m_stats; }

    /** @brief 重置统计数据 */
    void resetStatistics();

signals:
    /** @brief 求解完成信号,返回双连通分量数 */
    void solved(int componentCount);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_vertexCount = 0;
};
