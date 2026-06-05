#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 双连通分量与割点求解器
 *
 * 基于Tarjan算法计算图的双连通分量(Biconnected Components)
 * 和关节点(Articulation Points)，分析图的连通可靠性。
 */
class Biconnected6 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalSolved = 0; double avgProcessingTimeMs = 0.0; };

    explicit Biconnected6(QObject* parent = nullptr);

    /** @brief 设置顶点数量 */
    void setVertexCount(int count);

    /** @brief 添加无向边 */
    void addEdge(int from, int to);

    /** @brief 求解双连通分量 */
    QVector<QVector<int>> solve();

    /** @brief 获取割点(关节点)列表 */
    QVector<int> articulationPoints();

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 求解完成信号 */
    void solved(int componentCount);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_vertexCount = 0;
};
