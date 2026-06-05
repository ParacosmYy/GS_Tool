/**
 * @file GraphIsomorphism.h
 * @brief 图同构检测 — Weisfeiler-Lehman (WL) 测试
 *
 * 功能: 使用Weisfeiler-Lehman标签传播算法检测两个图的同构性,
 *       支持有向/无向图, 提供节点映射建议, 支持k-WL扩展。
 *       WL测试是图同构的必要非充分条件, 但实际中极少误判。
 *
 * 协作: PatternMatcher(模式匹配) / NetworkAnalyzer(网络分析) / ChemSimilarity(化学结构相似性)
 */

#pragma once

#include <QObject>
#include <QMap>
#include <QPair>

/**
 * @brief 图同构检测器 — Weisfeiler-Lehman 测试
 *
 * 通过多轮标签传播与重新编码检测两个图是否可能同构。
 * 算法复杂度: O(h * (|V| + |E|)), h 为迭代轮数。
 * 每轮将每个节点的标签更新为其邻居标签的多重集哈希值。
 */
class GraphIsomorphism : public QObject
{
    Q_OBJECT

public:
    /** @brief 图表示(邻接表) */
    using AdjacencyList = QVector<QVector<int>>;

    /** @brief 节点标签映射 */
    using LabelMap = QMap<int, int>;

    /** @brief 同构映射(节点索引对应) */
    using IsomorphismMap = QMap<int, int>;

    /** @brief 同构检测结果 */
    struct IsomorphismResult {
        bool possiblyIsomorphic = false;    ///< WL测试是否通过(可能同构)
        bool definitelyNonIsomorphic = false; ///< 是否确定不同构
        int iterationsUsed = 0;             ///< 实际使用的迭代轮数
        double confidence = 0.0;            ///< 置信度(0~1, 基于标签分布)
        IsomorphismMap nodeMapping;         ///< 建议的节点映射(若可能同构)
        QString failureReason;              ///< 不同构原因描述
    };

    /** @brief 图哈希签名 */
    struct GraphSignature {
        QVector<int> canonicalLabels;       ///< 规范化标签序列(排序后)
        QByteArray hashValue;               ///< 哈希值(MD5)
        int nodeCount = 0;                  ///< 节点数
        int edgeCount = 0;                  ///< 边数
    };

    /** @brief 统计信息 */
    struct Stats {
        int totalComparisons = 0;           ///< 累计比较次数
        int totalIsomorphicFound = 0;       ///< 累计发现同构次数
        int totalNonIsomorphicFound = 0;    ///< 累计发现不同构次数
        int totalSignaturesComputed = 0;    ///< 累计计算签名次数
        double avgProcessingTimeMs = 0.0;   ///< 平均处理耗时(ms)
    };

    explicit GraphIsomorphism(QObject* parent = nullptr);

    /**
     * @brief 检测两个图是否同构(WL测试)
     * @param adjList1 图1的邻接表
     * @param adjList2 图2的邻接表
     * @param maxIterations 最大迭代轮数(默认20)
     * @return 检测结果
     */
    IsomorphismResult checkIsomorphism(const AdjacencyList& adjList1,
                                        const AdjacencyList& adjList2,
                                        int maxIterations = 20);

    /**
     * @brief 计算图的WL签名(可用于快速预筛选)
     * @param adjList 邻接表
     * @param maxIterations 最大迭代轮数
     * @return 图签名
     */
    GraphSignature computeSignature(const AdjacencyList& adjList,
                                     int maxIterations = 20);

    /**
     * @brief 快速预筛选: 比较两个图的基本属性
     * @param adjList1 图1
     * @param adjList2 图2
     * @return true=属性匹配(可能同构), false=确定不同构
     */
    bool quickFilter(const AdjacencyList& adjList1,
                      const AdjacencyList& adjList2) const;

    /**
     * @brief 比较两个图签名的相似度
     * @param sig1 签名1
     * @param sig2 签名2
     * @return 相似度 (0~1)
     */
    double signatureSimilarity(const GraphSignature& sig1,
                                const GraphSignature& sig2) const;

    /**
     * @brief 批量检测: 从图集合中找出与目标图同构的所有图
     * @param target 目标图
     * @param candidates 候选图列表
     * @return 同构图索引列表
     */
    QVector<int> batchFindIsomorphic(const AdjacencyList& target,
                                      const QVector<AdjacencyList>& candidates);

    Stats stats() const;
    void resetStatistics();

private:
    /**
     * @brief 执行WL标签传播迭代
     * @param adjList 邻接表
     * @param labels 当前标签(输入/输出)
     * @return 本轮标签计数映射
     */
    QMap<int, int> wlIteration(const AdjacencyList& adjList,
                                QVector<int>& labels) const;

    /**
     * @brief 初始化节点标签(基于度数)
     * @param adjList 邻接表
     * @return 初始标签数组
     */
    QVector<int> initializeLabels(const AdjacencyList& adjList) const;

    /**
     * @brief 尝试构建节点映射
     * @param adjList1 图1
     * @param adjList2 图2
     * @param labels1 图1标签
     * @param labels2 图2标签
     * @return 映射结果
     */
    IsomorphismMap buildNodeMapping(const AdjacencyList& adjList1,
                                     const AdjacencyList& adjList2,
                                     const QVector<int>& labels1,
                                     const QVector<int>& labels2) const;

    /** @brief 计算标签分布哈希 */
    QByteArray computeLabelHash(const QVector<int>& labels) const;

    Stats m_stats;
    double m_timeSum = 0.0;
};
