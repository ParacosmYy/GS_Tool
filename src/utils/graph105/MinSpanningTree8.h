#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 最小生成树工具类
 *
 * 提供最小生成树求解功能，支持设置顶点数和带权边，
 * 计算连通图的最小权重生成树。
 */
class MinSpanningTree8 : public QObject {
    Q_OBJECT
public:
    /// 求解统计信息
    struct Stats {
        int totalSolves = 0;        ///< 总求解次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit MinSpanningTree8(QObject* parent = nullptr);

    /** @brief 设置图的顶点数量 */
    void setVertexCount(int count);

    /** @brief 添加一条带权无向边 */
    void addEdge(int from, int to, double weight);

    /** @brief 执行最小生成树求解 */
    void solve();

    /** @brief 获取最小生成树的总权重 */
    double totalWeight() const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 求解完成信号，返回总权重 */
    void solved(double weight);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_vertexCount = 0;
    QVector<QPair<QPair<int, int>, double>> m_edges;
    double m_totalWeight = 0.0;
};
