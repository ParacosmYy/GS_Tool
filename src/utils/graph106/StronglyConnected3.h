#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 强连通分量求解工具类
 *
 * 提配有向图强连通分量(SCC)的求解功能，
 * 基于Tarjan或Kosaraju算法实现。
 */
class StronglyConnected3 : public QObject {
    Q_OBJECT
public:
    /// 求解统计信息
    struct Stats {
        int totalSolves = 0;        ///< 总求解次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit StronglyConnected3(QObject* parent = nullptr);

    /** @brief 设置图的顶点数量 */
    void setVertexCount(int count);

    /** @brief 添加一条有向边 */
    void addEdge(int from, int to);

    /** @brief 执行强连通分量求解 */
    void solve();

    /** @brief 获取强连通分量数 */
    int componentCount() const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 求解完成信号，返回分量数 */
    void solved(int componentCount);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_vertexCount = 0;
    QVector<QPair<int, int>> m_edges;
    int m_componentCount = 0;
};
