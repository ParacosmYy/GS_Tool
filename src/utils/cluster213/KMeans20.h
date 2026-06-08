/**
 * @file KMeans20.h
 * @brief 核K均值聚类(RBF核距离+核k-means++质心初始化) — Kernel K-Means with RBF Kernelized Distance and Kernel K-Means++ Centroid Initialization
 *
 * 功能: 实现核K均值聚类算法，使用RBF核函数计算样本间距离，
 *       核k-means++初始化质心，支持非线性可分数据聚类。
 *
 * 协作: GaussianMixture18(高斯混合) / KMedoids17(K中心点) / OPTICS8(密度聚类)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 核K均值聚类(RBF核+k-means++初始化)
 */
class KMeans20 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numSamples = 0;
        int numClusters = 0;
        int dimensions = 0;
        int iterations = 0;
        double inertia = 0.0;
        double sigma = 1.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit KMeans20(QObject *parent = nullptr);
    ~KMeans20() override;

    /** @brief Set clustering parameters */
    void setParameters(int k, double sigma = 1.0, int maxIter = 300,
                       double tol = 1e-6);

    /** @brief Fit model to data using kernel k-means */
    void fit(const QVector<QVector<double>>& data);

    /** @brief Predict cluster for a new sample */
    int predict(const QVector<double>& sample) const;

    /** @brief Get cluster assignments for all training data */
    QVector<int> labels() const;

    /** @brief Compute RBF kernel between two vectors */
    double rbfKernel(const QVector<double>& a,
                     const QVector<double>& b) const;

    /** @brief Compute kernel distance from sample to cluster c */
    double kernelDistance(const QVector<double>& sample, int cluster) const;

    /** @brief Get intra-cluster inertia in kernel space */
    double inertia() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void fittingCompleted(int k, int iterations, double inertia, double timeMs);

private:
    int m_k = 3;
    double m_sigma = 1.0;
    int m_maxIter = 300;
    double m_tol = 1e-6;
    int m_dim = 0;
    int m_n = 0;

    QVector<QVector<double>> m_data;
    QVector<QVector<double>> m_kernelMatrix;
    QVector<int> m_labels;
    QVector<double> m_clusterSizes;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Precompute full kernel matrix */
    void computeKernelMatrix();

    /** @brief Initialize centroids via kernel k-means++ */
    void kernelInit();

    /** @brief Assign each sample to nearest cluster in kernel space */
    void assignClusters();

    /** @brief Compute cluster inertia in feature space */
    double computeInertia() const;
};
