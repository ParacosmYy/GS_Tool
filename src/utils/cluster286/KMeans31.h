/**
 * @file KMeans31.h
 * @brief K-means聚类(核密度估计初始化与马氏距离椭圆簇形状检测) — K-means with Kernel Density Estimation Initialization and Mahalanobis Distance for Elliptical Cluster Shape Detection
 *
 * 功能: 实现K-means聚类(KMeans)，采用核密度估计初始化(KDE initialization)
 *       与马氏距离(Mahalanobis distance)实现椭圆簇形状检测(elliptical cluster shape detection)。
 *
 * 协作: GaussianMixture34(高斯混合) / DBSCAN17(密度聚类) / KMeans30(K-means)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief K-means聚类(核密度估计初始化与马氏距离椭圆簇形状检测)
 */
class KMeans31 : public QObject {
    Q_OBJECT

public:
    /** @brief Cluster result with elliptical shape info */
    struct Cluster {
        QVector<double> mean;
        QVector<QVector<double>> covariance;
        QVector<QVector<double>> invCovariance;
        int count = 0;
        double mahalRadius = 0.0;
    };

    /** @brief Fit result */
    struct FitResult {
        QVector<Cluster> clusters;
        QVector<int> labels;
        int iterations = 0;
        double inertia = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numPoints = 0;
        int numClusters = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit KMeans31(QObject *parent = nullptr);
    ~KMeans31() override;

    void setK(int k);
    void setMaxIter(int iters);
    void setTolerance(double tol);
    void setBandwidth(double bw);

    /** @brief Fit K-means with KDE initialization and Mahalanobis distance */
    FitResult fit(const QVector<QVector<double>>& data);

    /** @brief Predict cluster for new samples */
    QVector<int> predict(const QVector<QVector<double>>& samples) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void fitDone(int n, int k, double inertia, double timeMs);

private:
    int m_k = 3;
    int m_maxIter = 300;
    double m_tol = 1e-6;
    double m_bandwidth = 0.0;
    Stats m_stats;
    double m_timeSum = 0.0;

    QVector<Cluster> m_clusters;
    int m_dim = 0;

    /** @brief Compute kernel density at a point using Gaussian kernel */
    double kdeAt(const QVector<QVector<double>>& data,
                 const QVector<double>& point, double bw) const;

    /** @brief Find high-density peaks for KDE-based initialization */
    QVector<QVector<double>> kdeInit(const QVector<QVector<double>>& data) const;

    /** @brief Compute Mahalanobis distance from point to cluster */
    double mahalanobis(const QVector<double>& x, const Cluster& c) const;

    /** @brief Compute Euclidean distance */
    double euclidean(const QVector<double>& a, const QVector<double>& b) const;

    /** @brief Update cluster covariance from assigned points */
    void updateCovariance(Cluster& c, const QVector<QVector<double>>& data,
                          const QVector<int>& labels, int cid);

    /** @brief Invert a positive-definite matrix (Gauss-Jordan) */
    QVector<QVector<double>> invertMatrix(const QVector<QVector<double>>& mat) const;
};
