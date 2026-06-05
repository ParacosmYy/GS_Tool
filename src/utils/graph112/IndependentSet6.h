#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 最大独立集求解器
 *
 * 在无向图中寻找最大独立集(互不相邻的顶点子集)，
 * 基于分支定界策略实现。
 */
class IndependentSet6 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats {
        int totalVertices = 0;       ///< 顶点总数
        int totalEdges = 0;          ///< 边总数
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit IndependentSet6(QObject* parent = nullptr);

    /** @brief 设置顶点数量 */
    void setVertexCount(int count);
    /** @brief 添加无向边 */
    void addEdge(int u, int v);
    /** @brief 执行求解 */
    void solve();
    /** @brief 获取最大独立集大小 */
    int setSize() const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 求解完成，返回独立集大小 */
    void solved(int setSize);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_vertexCount = 0;
    int m_setSize = 0;
    QVector<QPair<int, int>> m_edges;
};
