/**
 * @file KMeans33.h
 * @brief K均值聚类(均衡约束分配与最小簇大小强制实现公平划分的负载均衡聚类) — K-Means with Balanced Constraint Assignment and Minimum Cluster Size Enforcement for Fair Partitioning with Load Balancing
 *
 * 功能: 实现K均值聚类(K-means clustering)，采用均衡约束分配(balanced constraint assignment)
 *       与最小簇大小强制(minimum cluster size enforcement)实现公平划分的负载均衡聚类(fair partitioning with load balancing)。
 *
 * 协作: GaussianMixture37(高斯混合模型) / DBSCAN18(密度聚类) / KMeans32(K均值聚类)
 */
#pragma once

#include <QObject>
#include <QVector>

class KMeans33 : public QObject {
    Q_OBJECT

public:
    /** @brief Cluster center and statistics */
    struct Cluster {
        QVector<double> centroid;
        int count = 0;
        double intraVariance = 0.0;
    };

    /** @brief Fit result */
    struct FitResult {
        QVector<Cluster> clusters;
        QVector<int> assignments;
        double inertia = 0.0;
        bool balanced = false;
        int iterations = 0;
        double elapsedMs = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalFits = 0;
        int maxIterations = 0;
        double avgInertia = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit KMeans33(QObject *parent = nullptr);
    ~KMeans33() override;

    void setK(int k);
    void setMaxIterations(int iter);
    void setConvergenceThreshold(double tol);
    void setMinClusterSize(int minSize);
    void setBalanceRatio(double ratio);

    /** @brief Fit with balanced constraint assignment */
    FitResult fit(const QVector<QVector<double>>& data);

    /** @brief Predict cluster assignments */
    QVector<int> predict(const QVector<QVector<double>>& data) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void fitDone(int k, double inertia, int iterations, double timeMs);

private:
    int m_k = 3;
    int m_maxIter = 300;
    double m_tol = 1e-6;
    int m_minSize = 1;
    double m_balanceRatio = 1.0;
    int m_dims = 0;
    QVector<Cluster> m_clusters;
    Stats m_stats;
    double m_inertiaSum = 0.0;
    double m_timeSum = 0.0;

    /** @brief K-means++ initialization */
    void initialize(const QVector<QVector<double>>& data);

    /** @brief Balanced assignment with min-size enforcement */
    QVector<int> balancedAssign(const QVector<QVector<double>>& data) const;

    /** @brief Update centroids from assignments */
    void updateCentroids(const QVector<QVector<double>>& data,
                         const QVector<int>& assignments);

    /** @brief Squared Euclidean distance */
    double distSq(const QVector<double>& a, const QVector<double>& b) const;

    /** @brief Compute total inertia */
    double computeInertia(const QVector<QVector<double>>& data,
                          const QVector<int>& assignments) const;
};
