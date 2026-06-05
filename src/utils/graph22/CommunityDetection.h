/**
 * @file CommunityDetection.h
 * @brief 社区检测引擎 — Louvain算法实现模块度优化
 *
 * 功能: 基于Louvain启发式算法检测图中的社区结构，通过模块度最大化
 *       迭代合并节点，支持加权图、有向/无向图。输出层级社区划分。
 *
 * 协作: DataCorrelator(关联分析) / AnomalyDetector(异常检测)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>
#include <QMap>

/**
 * @brief 社区检测引擎 — Louvain模块度优化
 */
class CommunityDetection : public QObject {
    Q_OBJECT

public:
    /** @brief 图的边 */
    struct Edge {
        int source = 0;         ///< 源节点
        int target = 0;         ///< 目标节点
        double weight = 1.0;    ///< 边权重
    };

    /** @brief 社区信息 */
    struct Community {
        int id = 0;                         ///< 社区标识
        QList<int> members;                 ///< 成员节点列表
        double internalWeight = 0.0;        ///< 社区内部边权重和
        double totalDegree = 0.0;           ///< 社区总度数
        double modularity = 0.0;            ///< 社区局部模块度
    };

    /** @brief 检测结果 */
    struct Result {
        QList<Community> communities;       ///< 检测到的社区列表
        double totalModularity = 0.0;       ///< 总模块度(越大越好)
        int iterations = 0;                 ///< 迭代次数
        int levels = 0;                     ///< 层级数
    };

    /** @brief 运行统计 */
    struct Stats {
        int totalDetections = 0;            ///< 累计检测次数
        int totalNodesProcessed = 0;        ///< 累计处理节点数
        int totalEdgesProcessed = 0;        ///< 累计处理边数
        double avgProcessingTimeMs = 0.0;   ///< 平均处理时间(ms)
    };

    explicit CommunityDetection(QObject* parent = nullptr);

    /** @brief 设置解析度参数 @param gamma 解析度(默认1.0) */
    void setResolution(double gamma);

    /** @brief 从边列表构建图 @param edges 边列表 @param nodeCount 节点总数 */
    void buildGraph(const QList<Edge>& edges, int nodeCount);

    /** @brief 执行Louvain社区检测 @return 检测结果 */
    Result detect();

    /** @brief 获取节点的邻居及权重 @param node 节点 @return (邻居, 权重)列表 */
    QList<QPair<int, double>> getNeighbors(int node) const;

    /** @brief 计算模块度 @param communities 社区划分 @return 模块度值 */
    double computeModularity(const QList<Community>& communities) const;

    /** @brief 导出社区为GraphViz DOT格式 @return DOT字符串 */
    QString toDotFormat() const;

    /** @brief 清空图数据 */
    void clear();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 检测进度 @param iteration 当前迭代 @param modularity 当前模块度 */
    void progress(int iteration, double modularity);

    /** @brief 检测完成 @param result 最终结果 */
    void detectionComplete(const Result& result);

private:
    /** @brief 第一阶段: 局部移动节点优化模块度 @param nodeComm 节点->社区映射 @param commMembers 社区->成员映射 @return 是否有改进 */
    bool localMove(QVector<int>& nodeComm, QMap<int, QList<int>>& commMembers);

    /** @brief 计算将节点移入某社区的模块度增量 @param node 节点 @param targetComm 目标社区 @param nodeComm 当前社区映射 @return 增量 */
    double modularityGain(int node, int targetComm, const QVector<int>& nodeComm) const;

    /** @brief 第二阶段: 聚合社区为超节点，构建新图 @param nodeComm 节点->社区映射 @return 新图的边列表 */
    QList<Edge> aggregateCommunities(const QVector<int>& nodeComm);

    double m_resolution = 1.0;              ///< 解析度参数
    int m_nodeCount = 0;                    ///< 节点数
    double m_totalEdgeWeight = 0.0;         ///< 总边权重(2m)

    QVector<QList<QPair<int, double>>> m_adjList;    ///< 邻接表
    QVector<double> m_nodeDegree;                    ///< 节点加权度

    Stats m_stats;                          ///< 运行统计
    double m_timeSum = 0.0;                 ///< 处理时间累加器
};
