/**
 * @file KMeans26.h
 * @brief 核K均值(核化距离+迭代主化非线性可分聚类) — K-means with Kernel Trick via Kernelized Distance and Iterative Majorization for Non-linearly Separable Data
 *
 * 功能: 实现核K均值(Kernel K-means)算法，通过核化距离(kernelized
 *       distance)将数据映射到高维特征空间，配合迭代主化(iterative
 *       majorization)优化目标函数，处理非线性可分数据聚类。
 *
 * 协作: GaussianMixture27(高斯混合) / OPTICS11(OPTICS聚类) / DBSCAN12(密度聚类)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 核K均值(核化距离+迭代主化非线性可分聚类)
 */
class KMeans26 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numClusters = 0;
        int numPoints = 0;
        int numIterations = 0;
        double finalCost = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Kernel type enumeration */
    enum class KernelType { RBF, Polynomial, Linear };

    explicit KMeans26(QObject *parent = nullptr);
    ~KMeans26() override;

    /** @brief Set number of clusters */
    void setClusters(int k);

    /** @brief Set kernel type */
    void setKernelType(KernelType type);

    /** @brief Set RBF sigma parameter */
    void setSigma(double sigma);

    /** @brief Set polynomial degree */
    void setPolyDegree(int degree);

    /** @brief Set max iterations */
    void setMaxIterations(int iters);

    /** @brief Fit kernel k-means to data */
    bool fit(const QVector<QVector<double>>& data);

    /** @brief Predict cluster for a new sample */
    int predict(const QVector<double>& sample) const;

    /** @brief Get cluster assignments */
    QVector<int> labels() const;

    /** @brief Get cluster centroids in input space */
    QVector<QVector<double>> centroids() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void fittingCompleted(int clusters, double cost, double timeMs);

private:
    int m_k = 3;
    KernelType m_kernel = KernelType::RBF;
    double m_sigma = 1.0;
    int m_polyDegree = 2;
    int m_maxIter = 100;
    int m_dims = 0;
    int m_n = 0;

    QVector<QVector<double>> m_data;
    QVector<int> m_labels;
    QVector<QVector<double>> m_centroids;
    QVector<QVector<double>> m_kernelMatrix;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute kernel value between two samples */
    double kernelValue(const QVector<double>& a,
                       const QVector<double>& b) const;

    /** @brief Build full kernel matrix */
    void buildKernelMatrix();

    /** @brief Compute kernelized distance from point to cluster */
    double kernelDistance(int pointIdx, int cluster) const;

    /** @brief Initialize clusters via kernel k-means++ */
    void initialize();

    /** @brief Compute objective cost */
    double computeCost() const;
};
