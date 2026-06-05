/**
 * @file SteinerTree.h
 * @brief Steiner树近似算法 — 基于度量闭包的Steiner树求解
 *
 * 功能: 实现基于最短路径度量闭包的Steiner树2-近似算法，
 *       支持Prim MST、Floyd-Warshall全源最短路、Steiner顶点选取。
 *       适用于网络布线优化、串口设备最小连接拓扑。
 *
 * 协作: DataCorrelator(关联拓扑) / NetworkGraph(网络图分析)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>
#include <QSet>

/**
 * @brief Steiner树近似求解器 — 基于度量闭包
 */
class SteinerTree : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalTreesComputed = 0;            ///< 累计Steiner树计算次数
        int totalClosuresBuilt = 0;            ///< 累计度量闭包构建次数
        int totalMstComputed = 0;              ///< 累计MST计算次数
        double avgProcessingTimeMs = 0.0;      ///< 平均处理时间(ms)
    };

    /** @brief 图的带权边 */
    struct Edge {
        int u = 0;          ///< 起点编号
        int v = 0;          ///< 终点编号
        double weight = 0.0;///< 边权重
    };

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit SteinerTree(QObject* parent = nullptr);

    /**
     * @brief 构建带权无向图
     * @param edges 边列表
     * @param vertexCount 顶点数
     */
    void buildGraph(const QList<Edge>& edges, int vertexCount);

    /**
     * @brief 添加一条带权无向边
     * @param u 起点
     * @param v 终点
     * @param weight 权重
     */
    void addEdge(int u, int v, double weight);

    /**
     * @brief 计算Steiner树(2-近似)
     * @param terminals 终端顶点集合(必须连通的顶点)
     * @return Steiner树的边列表
     */
    QList<Edge> computeSteinerTree(const QSet<int>& terminals);

    /**
     * @brief 计算全源最短路径(Floyd-Warshall)
     * @return 距离矩阵 [i][j] = 最短距离
     */
    QVector<QVector<double>> floydWarshall() const;

    /**
     * @brief 构建度量闭包图(仅包含terminal顶点)
     * @param terminals 终端集合
     * @return 度量闭包的完全图边列表
     */
    QList<Edge> buildMetricClosure(const QSet<int>& terminals);

    /**
     * @brief Prim算法求MST
     * @param vertices 参与MST的顶点列表
     * @param distMatrix 距离矩阵
     * @return MST边列表
     */
    QList<Edge> primMST(const QVector<int>& vertices,
                        const QVector<QVector<double>>& distMatrix);

    /**
     * @brief 将度量闭包MST边映射回原图最短路径
     * @param closureEdges 度量闭包MST边
     * @param next Floyd-Warshall前驱矩阵
     * @return 原图中的路径边集合
     */
    QList<Edge> mapClosureToOriginal(const QList<Edge>& closureEdges,
                                     const QVector<QVector<int>>& next);

    /**
     * @brief 获取顶点数
     */
    int vertexCount() const { return m_vertexCount; }

    /**
     * @brief 获取边数
     */
    int edgeCount() const { return m_edges.size(); }

    /**
     * @brief 清空图
     */
    void clear();

    /**
     * @brief 获取统计信息
     */
    const Stats& stats() const { return m_stats; }

    /**
     * @brief 重置统计信息
     */
    void resetStatistics();

signals:
    /**
     * @brief Steiner树计算完成
     * @param treeEdges 树边数
     * @param totalWeight 树总权重
     */
    void treeComputed(int treeEdges, double totalWeight);

private:
    /**
     * @brief Floyd-Warshall同时返回前驱矩阵
     * @param dist 输出距离矩阵
     * @param next 输出前驱矩阵(路径重建用)
     */
    void floydWithPath(QVector<QVector<double>>& dist,
                       QVector<QVector<int>>& next) const;

    QVector<QVector<double>> m_adjMatrix;  ///< 邻接矩阵
    QList<Edge> m_edges;                    ///< 边列表
    int m_vertexCount = 0;                  ///< 顶点数

    Stats m_stats;
    double m_timeSum = 0.0;                 ///< 处理时间累加器
};
