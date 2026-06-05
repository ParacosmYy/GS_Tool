#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 层次聚类算法实现 (11种链接策略)
 *
 * 提供自底向上的层次聚类，支持单链接、全链接、平均链接等多种策略，
 * 输出树状图结构。
 */
class HierarchicalCluster11 : public QObject {
    Q_OBJECT
public:
    /// 合并步骤记录
    struct MergeStep {
        int clusterA = 0;          ///< 合并簇A索引
        int clusterB = 0;          ///< 合并簇B索引
        double distance = 0.0;     ///< 合并距离
        int newSize = 0;           ///< 合并后簇大小
    };

    /// 统计信息结构
    struct Stats {
        int totalClusteringRuns = 0;   ///< 总聚类运行次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
        int totalMergeSteps = 0;       ///< 总合并步骤数
    };

    explicit HierarchicalCluster11(QObject* parent = nullptr);
    Stats stats() const { return m_stats; }
    void resetStatistics();

    /**
     * @brief 执行层次聚类
     * @param distanceMatrix 距离矩阵
     * @param linkageType 链接策略: 0=单链接 1=全链接 2=平均链接
     * @return 合并步骤序列（树状图）
     */
    QVector<MergeStep> fit(const QVector<QVector<double>>& distanceMatrix, int linkageType = 2);

    /**
     * @brief 从树状图切割指定数量的簇
     * @param mergeSteps 合并步骤序列
     * @param k 目标簇数
     * @return 每个样本的簇标签
     */
    QVector<int> cutTree(const QVector<MergeStep>& mergeSteps, int k) const;

    /**
     * @brief 计算轮廓系数评估聚类质量
     * @param labels 簇标签
     * @param distanceMatrix 距离矩阵
     * @return 平均轮廓系数 [-1, 1]
     */
    double silhouetteScore(const QVector<int>& labels, const QVector<QVector<double>>& distanceMatrix) const;

    /**
     * @brief 获取合并距离序列（用于肘部法则）
     * @return 各合并步骤的距离
     */
    QVector<double> mergeDistances() const { return m_mergeDistances; }

signals:
    /// 聚类完成信号
    void clusteringCompleted(int clusterCount);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    QVector<double> m_mergeDistances;
};
