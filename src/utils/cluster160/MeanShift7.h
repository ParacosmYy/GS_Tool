/**
 * @file MeanShift7.h
 * @brief 均值漂移聚类(自适应KNN带宽) — Mean Shift Clustering with Adaptive KNN Bandwidth
 *
 * 功能: 基于核密度估计的均值漂移聚类，支持任意形状簇发现。
 *       使用K近邻距离自适应计算带宽，无需手动指定簇数。
 *       支持高斯核和截断核，自动合并收敛中心。
 *
 * 协作: DBSCAN8(密度聚类) / KMedoids13(PAM聚类) / GaussianMixture(高斯混合)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 均值漂移聚类器，KNN自适应带宽
 */
class MeanShift7 : public QObject {
    Q_OBJECT

public:
    /** @brief 核函数类型 */
    enum class KernelType {
        Gaussian,   ///< 高斯核
        Flat        ///< 截断核(Uniform)
    };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalClusterOps = 0;       ///< 累计聚类操作次数
        quint64 clusterCount = 0;          ///< 最终簇数
        quint64 totalIterations = 0;       ///< 累计迭代次数
        double avgProcessingTimeMs = 0.0;  ///< 平均处理耗时(ms)
    };

    explicit MeanShift7(QObject* parent = nullptr);

    /**
     * @brief 设置K近邻数(用于自适应带宽)
     * @param k 近邻数，>= 3
     */
    void setK(int k);

    /**
     * @brief 设置核函数类型
     * @param type 核类型
     */
    void setKernel(KernelType type);

    /**
     * @brief 设置收敛阈值
     * @param threshold 位移小于此值时停止迭代
     */
    void setConvergenceThreshold(double threshold);

    /**
     * @brief 设置最大迭代次数
     * @param maxIter 最大迭代次数
     */
    void setMaxIterations(int maxIter);

    /**
     * @brief 设置簇合并距离阈值
     * @param threshold 中心距离小于此值时合并
     */
    void setMergeThreshold(double threshold);

    /**
     * @brief 执行均值漂移聚类
     * @param data 输入数据，每个元素为特征向量
     * @return 每个样本的簇标签
     */
    QVector<int> fit(const QVector<QVector<double>>& data);

    /**
     * @brief 获取收敛后的簇中心
     */
    QVector<QVector<double>> clusterCenters() const { return m_centers; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 聚类完成 @param clusters 簇数 @param iterations 总迭代次数 */
    void fitCompleted(int clusters, int iterations);

private:
    /** @brief 计算两点欧氏距离 */
    double euclidean(const QVector<double>& a, const QVector<double>& b) const;

    /** @brief 计算单点的自适应带宽(K近邻平均距离) */
    double adaptiveBandwidth(int pointIdx, const QVector<QVector<double>>& data) const;

    /** @brief 单次均值漂移迭代 */
    QVector<double> shiftPoint(const QVector<double>& point,
                                const QVector<QVector<double>>& data,
                                double bandwidth) const;

    /** @brief 合并距离过近的簇中心 */
    void mergeCenters();

    /** @brief 为每个点分配最近簇标签 */
    void assignLabels(const QVector<QVector<double>>& data);

    int m_k = 10;
    KernelType m_kernel = KernelType::Gaussian;
    double m_convergenceThreshold = 1e-4;
    int m_maxIterations = 300;
    double m_mergeThreshold = 0.1;

    QVector<QVector<double>> m_centers;    ///< 收敛后的簇中心
    QVector<int> m_labels;                 ///< 每个点的簇标签

    Stats m_stats;
    double m_timeSum = 0.0;
};
