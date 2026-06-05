#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 最大独立集求解工具类
 *
 * 提供图的最大独立集求解功能，支持设置顶点数和边，
 * 寻找图中最大的互不相邻顶点子集。
 */
class IndependentSet5 : public QObject {
    Q_OBJECT
public:
    /// 求解统计信息
    struct Stats {
        int totalSolves = 0;        ///< 总求解次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit IndependentSet5(QObject* parent = nullptr);

    /** @brief 设置图的顶点数量 */
    void setVertexCount(int count);

    /** @brief 添加一条无向边 */
    void addEdge(int from, int to);

    /** @brief 执行最大独立集求解 */
    void solve();

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 求解完成信号，返回独立集大小 */
    void solved(int setSize);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_vertexCount = 0;
    QVector<QPair<int, int>> m_edges;
};
