/**
 * @file GraphPartition.h
 * @brief 图分割引擎 — Kernighan-Lin/FM细化/多层递归二分
 *
 * 功能: 实现Kernighan-Lin双向分割、Fiduccia-Mattheyses细化、
 *       多层递归二分，最小化边割数，支持加权图。
 *
 * 协作: DataClassifier(数据分类) / CycleDetector(环检测)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>

/**
 * @brief 图分割引擎
 */
class GraphPartition : public QObject {
    Q_OBJECT

public:
    /** @brief 分割方法 */
    enum class PartitionMethod {
        KernighanLin,          ///< Kernighan-Lin双向分割
        FiducciaMattheyses,    ///< FM细化
        MultilevelBisection    ///< 多层递归二分
    };
    Q_ENUM(PartitionMethod)

    /** @brief 图边 */
    struct Edge {
        int source = 0;            ///< 源顶点
        int target = 0;            ///< 目标顶点
        double weight = 1.0;       ///< 边权重
    };

    /** @brief 分割结果 */
    struct PartitionResult {
        QVector<int> partition;    ///< 每个顶点的分区编号(0或1)
        int edgeCut = 0;           ///< 边割数
        double imbalance = 0.0;    ///< 不平衡度
        int iterations = 0;        ///< 迭代次数
    };

    /** @brief 统计 */
    struct Stats {
        quint64 totalPartitions = 0;       ///< 累计分割次数
        quint64 totalVerticesProcessed = 0;///< 累计处理顶点数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
        int     bestEdgeCut = 0;           ///< 历史最小边割
    };

    explicit GraphPartition(QObject* parent = nullptr);

    /** @brief 设置分割方法 @param method 方法 */
    void setMethod(PartitionMethod method);

    /** @brief 设置最大迭代次数 @param maxIter 最大迭代 */
    void setMaxIterations(int maxIter);

    /** @brief 设置平衡约束 @param tolerance 不平衡容忍度(0~1) */
    void setBalanceTolerance(double tolerance);

    /** @brief 执行图分割 @param numVertices 顶点数 @param edges 边列表 @return 分割结果 */
    PartitionResult partition(int numVertices, const QList<Edge>& edges);

    /** @brief 多路分割(K=2^n) @param numVertices 顶点数 @param edges 边列表 @param numParts 目标分区数 @return 分割结果 */
    PartitionResult multiWayPartition(int numVertices,
                                      const QList<Edge>& edges,
                                      int numParts);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 分割完成 @param edgeCut 边割数 @param iterations 迭代次数 */
    void partitionComplete(int edgeCut, int iterations);

    /** @brief 迭代进展 @param iteration 当前迭代 @param currentCut 当前边割 */
    void iterationProgress(int iteration, int currentCut);

private:
    PartitionResult kernighanLin(int n, const QList<Edge>& edges);
    PartitionResult fiducciaMattheyses(int n, const QList<Edge>& edges);
    PartitionResult multilevelBisection(int n, const QList<Edge>& edges);
    QVector<int> initialBisection(int n) const;
    int computeEdgeCut(const QVector<int>& part,
                       const QList<Edge>& edges) const;
    double computeImbalance(const QVector<int>& part) const;

    PartitionMethod m_method;       ///< 分割方法
    int m_maxIter;                  ///< 最大迭代次数
    double m_tolerance;             ///< 平衡容忍度

    Stats m_stats;
    double m_timeSum = 0.0;         ///< 处理时间累加器
};
