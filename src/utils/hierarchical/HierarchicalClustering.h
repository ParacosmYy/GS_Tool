/**
 * @file HierarchicalClustering.h
 * @brief 层次聚类(Hierarchical Clustering)
 */

#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @class HierarchicalClustering
 * @brief 层次聚类 — 凝聚式/分裂式层次聚类
 *
 * 支持多种链接策略(单链接/全链接/平均链接/Ward)、
 * 树状图生成、最优切割。
 * 适用于数据探索、聚类分析、分类体系构建等场景。
 */
class HierarchicalClustering : public QObject
{
    Q_OBJECT

public:
    /** @brief 链接策略 */
    enum Linkage {
        Single,     /**< 单链接(最近邻) */
        Complete,   /**< 全链接(最远邻) */
        Average,    /**< 平均链接 */
        Ward        /**< Ward最小方差 */
    };
    Q_ENUM(Linkage)

    /** @brief 合并步骤 */
    struct MergeStep {
        int cluster1;    /**< 合并簇1索引 */
        int cluster2;    /**< 合并簇2索引 */
        double distance; /**< 合并距离 */
        int newSize;     /**< 合并后大小 */
    };

    /** @brief 统计信息 */
    struct Stats {
        int totalClustered = 0; /**< 总聚类次数 */
        int totalPoints = 0;    /**< 总数据点数 */
        double avgProcessingTimeMs = 0.0; /**< 平均处理时间(ms) */
    };

    /** @brief 构造函数 */
    explicit HierarchicalClustering(QObject* parent = nullptr);

    /**
     * @brief 凝聚式层次聚类
     * @param data 数据点(每行一个点, 各列为特征)
     * @param linkage 链接策略
     * @return 合并步骤序列
     */
    QVector<MergeStep> fit(const QVector<QVector<double>>& data,
                             Linkage linkage = Ward);

    /**
     * @brief 根据合并步骤获取聚类结果
     * @param merges 合并步骤
     * @param nClusters 目标簇数
     * @return 每个点的簇标签
     */
    QVector<int> getClusters(const QVector<MergeStep>& merges,
                               int nClusters) const;

    /**
     * @brief 自动选择最优簇数(不一致系数法)
     * @param merges 合并步骤
     * @return 推荐簇数
     */
    int optimalClusterCount(const QVector<MergeStep>& merges) const;

    /**
     * @brief 计算两个点之间的欧氏距离
     * @param a 点a
     * @param b 点b
     * @return 距离
     */
    static double euclidean(const QVector<double>& a,
                              const QVector<double>& b);

    /** @brief 获取统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 聚类完成信号 */
    void clusteringCompleted(int points, int steps);

private:
    double clusterDistance(const QVector<int>& c1, const QVector<int>& c2,
                            const QVector<QVector<double>>& data,
                            Linkage linkage) const;

    Stats m_stats;
    double m_timeSum;
};
