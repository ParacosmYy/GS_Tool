#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 最小生成树求解器
 *
 * 基于Kruskal或Prim算法求解加权无向图的最小生成树，
 * 返回总权重和边集合。
 */
class MinSpanningTree9 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats {
        int totalVertices = 0;       ///< 顶点总数
        int totalEdges = 0;          ///< 边总数
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit MinSpanningTree9(QObject* parent = nullptr);

    /** @brief 设置顶点数量 */
    void setVertexCount(int count);
    /** @brief 添加带权无向边 */
    void addEdge(int u, int v, double weight);
    /** @brief 执行最小生成树求解 */
    void solve();
    /** @brief 获取最小生成树总权重 */
    double totalWeight() const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 求解完成，返回总权重 */
    void solved(double weight);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_vertexCount = 0;
    double m_totalWeight = 0.0;
    QVector<QPair<int, int>> m_edges;
};
