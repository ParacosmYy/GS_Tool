/**
 * @file Agglomerative7.h
 * @brief 层次凝聚聚类(Ward最小方差+共表相关系数) — Agglomerative Clustering with Ward's Minimum Variance and Cophenetic Correlation
 *
 * 功能: 实现层次凝聚聚类算法，支持Ward最小方差 linkage、
 *       Lance-Williams递归更新、共表相关系数评估。
 *
 * 协作: MeanShift8(均值漂移) / KMedoids14(K-Medoids) / DBSCAN9(DBSCAN)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 层次凝聚聚类器(Ward最小方差)
 */
class Agglomerative7 : public QObject {
    Q_OBJECT

public:
    /** @brief 距离链接方法 */
    enum Linkage { Ward = 0, Single = 1, Complete = 2, Average = 3 };

    /** @brief 合并记录 */
    struct MergeStep {
        int clusterA = -1;       ///< 合并簇A索引
        int clusterB = -1;       ///< 合并簇B索引
        double distance = 0.0;   ///< 合并距离
        int newSize = 0;         ///< 合并后簇大小
    };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalRuns = 0;            ///< 累计运行次数
        int numClusters = 0;              ///< 最近聚类数
        int numSamples = 0;               ///< 样本数
        double copheneticCorrelation = 0.0;///< 共表相关系数
        double avgProcessingTimeMs = 0.0; ///< 平均耗时(ms)
    };

    explicit Agglomerative7(QObject *parent = nullptr);
    ~Agglomerative7() override;

    void setLinkage(Linkage method);
    void setNumClusters(int k);

    /**
     * @brief 执行层次凝聚聚类
     * @param data 数据集(每行一个样本)
     * @return 每个样本的簇标签
     */
    QVector<int> fit(const QVector<QVector<double>>& data);

    /** @brief 获取合并历史(树状图数据) */
    QVector<MergeStep> mergeHistory() const;

    /** @brief 计算共表相关系数 */
    double copheneticCorrelation() const;

    /** @brief 获取指定层次的标签 */
    QVector<int> cutTree(int k) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void clusteringCompleted(int numClusters, double cophCorr);
    void mergeProgress(int step, int total);

private:
    /** @brief 计算Ward距离增量 */
    double wardDistance(const QVector<int>& a, const QVector<int>& b,
                       const QVector<QVector<double>>& data) const;

    /** @brief Lance-Williams递归更新距离 */
    double lanceWilliamsUpdate(double dij, double dik, double djk,
                               int ni, int nj, int nk) const;

    /** @brief 计算簇中心 */
    QVector<double> clusterCentroid(
        const QVector<int>& indices,
        const QVector<QVector<double>>& data) const;

    /** @brief 欧氏距离平方 */
    static double euclideanSq(const QVector<double>& a,
                              const QVector<double>& b);

    Linkage m_linkage = Ward;
    int m_numClusters = 2;

    QVector<MergeStep> m_merges;        ///< 合并历史
    QVector<QVector<double>> m_data;    ///< 缓存数据
    QVector<int> m_labels;

    Stats m_stats;
    double m_timeSum = 0.0;
};
