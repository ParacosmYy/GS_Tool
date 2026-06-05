#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 最小支配集求解器
 *
 * 寻找图的最小支配集(Dominating Set)，使得图中每个顶点
 * 要么在支配集中，要么与支配集中的顶点相邻。
 */
class DominatingSet7 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalSolved = 0; double avgProcessingTimeMs = 0.0; };

    explicit DominatingSet7(QObject* parent = nullptr);

    /** @brief 设置顶点数量 */
    void setVertexCount(int count);

    /** @brief 添加无向边 */
    void addEdge(int from, int to);

    /** @brief 求解最小支配集 */
    QVector<int> solve();

    /** @brief 获取支配集覆盖大小 */
    int coverSize();

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 求解完成信号 */
    void solved(int setSize);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_vertexCount = 0;
};
