/**
 * @file AStarSearch.h
 * @brief A*图搜索算法 — 启发式最短路径搜索
 *
 * 功能: 实现A*算法在加权图中搜索最短路径, 支持自定义
 *       启发函数和双向搜索模式。使用优先队列优化节点扩展。
 *
 * 协作: GraphColoring(图着色) / FloydWarshall(全源最短路)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>
#include <QHash>
#include <QPair>
#include <functional>

/**
 * @brief A*图搜索算法 — 启发式最短路径搜索
 */
class AStarSearch : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalSearches = 0;        ///< 累计搜索次数
        double  avgNodesVisited = 0.0;    ///< 平均访问节点数
        double  avgPathLength = 0.0;      ///< 平均路径长度
        double  avgProcessingTimeMs = 0.0;///< 平均处理时间(ms)
    };

    /** @brief 默认构造函数 @param parent 父对象 */
    explicit AStarSearch(QObject* parent = nullptr);

    /** @brief 边结构: 起点、终点、权重 */
    struct Edge {
        QString from;   ///< 起点名称
        QString to;     ///< 终点名称
        double  weight; ///< 边权重(必须为正数)
    };

    /**
     * @brief 设置图结构
     * @param nodes 节点名称列表
     * @param edges 边列表: 每项包含起点、终点和权重
     *
     * 图为有向图; 若需要无向图请同时添加反向边。
     * 边权重必须为正数(A*要求非负权重)。
     */
    void setGraph(const QVector<QString>& nodes,
                  const QVector<Edge>& edges);

    /**
     * @brief 设置启发函数
     * @param fn 启发函数, 参数为(当前节点, 目标节点), 返回估计距离
     *
     * 启发函数必须满足可容许性(不高估实际距离)以保证最优性。
     * 若不设置则使用零启发(退化为Dijkstra算法)。
     */
    void setHeuristic(
        std::function<double(const QString&, const QString&)> fn);

    /**
     * @brief 执行A*搜索
     * @param start 起点名称
     * @param goal 终点名称
     * @return 从start到goal的最短路径(包含两端), 若不可达则返回空
     */
    QVector<QString> search(const QString& start, const QString& goal);

    /**
     * @brief 启用/禁用双向搜索
     * @param enabled true启用双向A*, false使用单向A*
     *
     * 双向A*同时从起点和终点开始搜索, 在中间相遇时合并路径。
     * 对大规模图搜索效率更高。
     */
    void setBidirectional(bool enabled);

    /** @brief 获取统计 @return 统计信息常引用 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief 搜索完成 @param path 路径 @param cost 路径总代价 */
    void searchCompleted(const QVector<QString>& path, double cost);

    /** @brief 节点被访问 @param node 节点名称 @param gScore 当前g值 */
    void nodeVisited(const QString& node, double gScore);

private:
    /**
     * @brief 单向A*搜索核心
     * @param start 起点
     * @param goal 终点
     * @param forward true正向搜索, false反向搜索
     * @return {路径, 代价, 已访问节点数}
     */
    QPair<QVector<QString>, double> searchSingle(
        const QString& start, const QString& goal, bool forward = true) const;

    /**
     * @brief 重建路径(从cameFrom映射回溯)
     * @param cameFrom 前驱映射
     * @param current 终点
     * @return 路径列表
     */
    QVector<QString> reconstructPath(
        const QHash<QString, QString>& cameFrom,
        const QString& current) const;

    QHash<QString, QHash<QString, double>> m_adjForward;  ///< 正向邻接表
    QHash<QString, QHash<QString, double>> m_adjBackward; ///< 反向邻接表
    QHash<QString, bool> m_nodeExists;                     ///< 节点存在性
    bool m_bidirectional;                                   ///< 双向搜索标志

    std::function<double(const QString&, const QString&)> m_heuristic; ///< 启发函数

    Stats  m_stats;                ///< 统计信息
    double m_timeSum;              ///< 处理时间累加器
    double m_nodesVisitedSum;      ///< 已访问节点数累加器
    double m_pathLengthSum;        ///< 路径长度累加器
};
