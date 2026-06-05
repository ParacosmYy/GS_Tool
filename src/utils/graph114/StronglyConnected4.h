#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 强连通分量求解器
 *
 * 基于Tarjan或Kosaraju算法求解有向图的强连通分量，
 * 用于依赖分析和图的结构化简。
 */
class StronglyConnected4 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats {
        int totalVertices = 0;       ///< 顶点总数
        int totalEdges = 0;          ///< 边总数
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit StronglyConnected4(QObject* parent = nullptr);

    /** @brief 设置顶点数量 */
    void setVertexCount(int count);
    /** @brief 添加有向边 */
    void addEdge(int from, int to);
    /** @brief 执行强连通分量分解 */
    void solve();
    /** @brief 获取强连通分量数 */
    int componentCount() const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 求解完成，返回分量数 */
    void solved(int componentCount);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_vertexCount = 0;
    int m_componentCount = 0;
    QVector<QPair<int, int>> m_edges;
};
