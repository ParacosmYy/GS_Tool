/**
 * @file MeanShift9.h
 * @brief 均值漂移聚类(可变带宽KNN+吸引盆地合并) — Mean Shift Clustering with Variable Bandwidth via K-Nearest-Neighbor Distance and Basin of Attraction Merging
 *
 * 功能: 实现均值漂移聚类算法，支持基于KNN距离的可变带宽、
 *       吸引盆地(Basin of Attraction)合并、多维度数据和多核函数。
 *
 * 协作: KMeans17(K均值) / DBSCAN10(密度聚类) / GaussianMixture14(高斯混合)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 均值漂移聚类器(可变带宽KNN+吸引盆地合并)
 */
class MeanShift9 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalRuns = 0;
        int numPoints = 0;
        int numClusters = 0;
        int iterationsUsed = 0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief 核函数类型 */
    enum Kernel { Gaussian, Epanechnikov, Flat };

    explicit MeanShift9(QObject *parent = nullptr);
    ~MeanShift9() override;

    void setBandwidth(double h);
    void setKnnK(int k);
    void setMaxIterations(int iter);
    void setTolerance(double tol);
    void setMergeThreshold(double t);
    void setKernel(Kernel k);

    /** @brief 执行均值漂移聚类，返回每个点的簇标签 */
    QVector<int> fit(const QVector<QVector<double>>& data);

    /** @brief 获取模式点(聚类中心) */
    QVector<QVector<double>> modes() const { return m_modes; }

    /** @brief 获取每个点的局部带宽(KNN距离) */
    QVector<double> bandwidths() const { return m_bandwidths; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void clusteringCompleted(int clusters, int iterations, double timeMs);

private:
    double m_bandwidth = 1.0;
    int m_knnK = 10;
    int m_maxIterations = 300;
    double m_tolerance = 1e-4;
    double m_mergeThreshold = 0.1;
    Kernel m_kernel = Gaussian;

    QVector<QVector<double>> m_modes;
    QVector<double> m_bandwidths;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Euclidean distance between two points */
    double distance(const QVector<double>& a, const QVector<double>& b) const;

    /** @brief Compute per-point bandwidth via KNN distance */
    void computeKnnBandwidths(const QVector<QVector<double>>& data);

    /** @brief Kernel weight given distance and bandwidth */
    double kernelWeight(double dist, double h) const;

    /** @brief Single point mean shift iteration */
    QVector<double> shiftPoint(const QVector<double>& point,
                               const QVector<QVector<double>>& data,
                               double h) const;

    /** @brief Merge nearby modes into clusters */
    QVector<int> mergeModes(double threshold);
};
