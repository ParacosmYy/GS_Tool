/**
 * @file ChinesePostman.h
 * @brief 中国邮路算法 — 欧拉增广求解无向/有向图的最短遍历路径
 *
 * 功能: 对任意连通图执行最小代价奇度配对，构造欧拉增广图，
 *       输出经过所有边至少一次的最短闭路。
 *       支持Floyd-Warshall全源最短路 + 完美匹配搜索。
 *
 * 协作: WeightedUnionFind(连通性判断) / DataTransformer(路径序列化)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>

/**
 * @brief 中国邮路算法引擎 — 欧拉增广求解最短遍历路径
 */
class ChinesePostman : public QObject {
    Q_OBJECT

public:
    /** @brief 边结构 */
    struct Edge {
        int from = 0;           ///< 起始顶点
        int to = 0;             ///< 目标顶点
        double weight = 1.0;    ///< 边权重(代价)
        bool directed = false;  ///< 是否有向边
    };

    /** @brief 求解结果 */
    struct PostmanResult {
        QList<int> route;           ///< 顶点序列(闭路)
        double totalCost = 0.0;     ///< 总遍历代价
        int augmentedEdges = 0;     ///< 增广的额外边数
        bool isEulerian = false;    ///< 原图是否已是欧拉图
        bool valid = false;         ///< 求解是否成功
    };

    /** @brief 运行时统计信息 */
    struct Stats {
        int totalSolves = 0;                ///< 累计求解次数
        int totalNodesProcessed = 0;        ///< 累计处理顶点数
        int totalEdgesProcessed = 0;        ///< 累计处理边数
        double avgProcessingTimeMs = 0.0;   ///< 平均处理耗时(ms)
    };

    /** @brief 构造函数 @param parent 父对象 */
    explicit ChinesePostman(QObject* parent = nullptr);

    /** @brief 设置图的顶点数 @param n 顶点数 */
    void setNodeCount(int n);

    /** @brief 添加边 @param edge 边 */
    void addEdge(const Edge& edge);

    /** @brief 清空图 */
    void clear();

    /** @brief 求解中国邮路 @return 遍历结果 */
    PostmanResult solve();

    /** @brief 检查是否为欧拉图 @return 是否所有顶点度数均为偶数 */
    bool isEulerian() const;

    /** @brief 获取当前统计 @return 统计常量引用 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计计数器 */
    void resetStatistics();

signals:
    /** @brief 求解完成 @param cost 总代价 @param steps 路径步数 */
    void solved(double cost, int steps);

private:
    /** @brief Floyd-Warshall全源最短路 */
    void floydWarshall();

    /** @brief 查找奇度顶点 */
    QVector<int> findOddDegreeNodes() const;

    /** @brief 最小权重完美匹配(暴力搜索) */
    QVector<QPair<int,int>> minimumWeightMatching(const QVector<int>& oddNodes);

    /** @brief 构造欧拉增广邻接表 */
    QVector<QList<QPair<int,double>>> buildAugmentedGraph(
        const QVector<QPair<int,int>>& matching);

    /** @brief Hierholzer算法找欧拉回路 */
    QList<int> findEulerCircuit(
        QVector<QList<QPair<int,double>>>& adj, int start);

    int m_nodeCount;                                    ///< 顶点数
    QList<Edge> m_edges;                                ///< 边列表
    QVector<QVector<double>> m_dist;                    ///< 最短路径矩阵
    QVector<QVector<int>> m_next;                       ///< 路径重建矩阵
    bool m_dirty;                                       ///< 是否需要重建最短路

    Stats m_stats;                                      ///< 运行时统计
    double m_timeSum = 0.0;                             ///< 累计耗时(ms)
};
