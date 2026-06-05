#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief HierarchicalCluster12 - 层次聚类第12代实现
 *
 * 提供凝聚式/分裂式层次聚类，支持多种链接准则
 * （单链接、全链接、平均链接、Ward法）及树状图生成。
 */
class HierarchicalCluster12 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalMergeOps = 0; double avgProcessingTimeMs = 0.0; };
    explicit HierarchicalCluster12(QObject* parent = nullptr);
    Stats stats() const { return m_stats; }
    void resetStatistics();

    /**
     * @brief 执行凝聚式层次聚类
     * @param distanceMatrix 距离矩阵
     * @param linkage 链接准则 (single/complete/average/ward)
     * @return 每步合并记录 (合并的两个簇索引, 合并距离)
     */
    QVector<QPair<QPair<int, int>, double>> agglomerative(
        const QVector<QVector<double>>& distanceMatrix, const QString& linkage);

    /**
     * @brief 根据合并记录在指定距离处截断获取聚类
     * @param mergeSteps 合并步骤记录
     * @param cutDistance 截断距离阈值
     * @return 各样本的聚类标签
     */
    QVector<int> cutDendrogram(const QVector<QPair<QPair<int, int>, double>>& mergeSteps,
                               double cutDistance);

    /**
     * @brief 计算簇间Ward距离
     * @param clusterA 簇A中的数据点索引
     * @param clusterB 簇B中的数据点索引
     * @param centroids 各簇质心
     * @return Ward距离值
     */
    double wardDistance(const QVector<int>& clusterA, const QVector<int>& clusterB,
                       const QVector<QVector<double>>& centroids);

signals:
    void clusteringCompleted(int clusterCount);

private:
    Stats m_stats; double m_timeSum = 0.0;
};
