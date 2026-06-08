/**
 * @file KMeans23.h
 * @brief K均值聚类(Canopy预聚类初始化+三角不等式加速) — K-Means with Canopy Pre-Clustering for Initial Center Estimation and Triangle Inequality Acceleration
 *
 * 功能: 实现K-means聚类算法，采用Canopy预聚类(canopy pre-clustering)进行初始中心估计，
 *       利用三角不等式(triangle inequality)跳过不必要的距离计算以加速收敛。
 *
 * 协作: GaussianMixture22(高斯混合) / DBSCAN13(密度聚类) / KMedoids18(K中心点)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief K均值聚类(Canopy预聚类初始化+三角不等式加速)
 */
class KMeans23 : public QObject {
    Q_OBJECT

public:
    /** @brief Cluster result for a single point */
    struct Assignment {
        int clusterId = -1;
        double distance = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numClusters = 0;
        int numDimensions = 0;
        int numPoints = 0;
        int numIterations = 0;
        int numDistSaved = 0;
        double inertia = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit KMeans23(QObject *parent = nullptr);
    ~KMeans23() override;

    /** @brief Configure cluster count and canopy thresholds */
    void setParameters(int k, double canopyT1 = 0.0, double canopyT2 = 0.0);

    /** @brief Fit model on data matrix [n x d] */
    bool fit(const QVector<QVector<double>>& data, int maxIter = 300, double tol = 1e-4);

    /** @brief Predict cluster for a single point */
    int predict(const QVector<double>& point) const;

    /** @brief Get all cluster assignments */
    QVector<Assignment> assignments() const;

    /** @brief Get cluster centroids */
    QVector<QVector<double>> centroids() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void iterationCompleted(int iter, double inertia);
    void fitCompleted(int clusters, double inertia, double timeMs);

private:
    int m_k = 3;
    int m_d = 2;
    double m_canopyT1 = 0.0;
    double m_canopyT2 = 0.0;

    QVector<QVector<double>> m_centroids;
    QVector<Assignment> m_assignments;

    // Triangle inequality acceleration caches
    QVector<double> m_lowerBounds;   // [n*k] per-point per-cluster lower bound
    QVector<double> m_upperBound;    // [n] per-point upper bound
    QVector<double> m_clusterDist;   // [k*k] inter-cluster distances
    QVector<double> m_centerShift;   // [k] centroid movement

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Run Canopy pre-clustering to estimate initial centers */
    QVector<int> canopyPrecluster(const QVector<QVector<double>>& data) const;

    /** @brief Initialize centroids from canopy centers */
    void initCentroids(const QVector<QVector<double>>& data, const QVector<int>& canopies);

    /** @brief Compute squared Euclidean distance */
    double squaredDist(const QVector<double>& a, const QVector<double>& b) const;

    /** @brief Compute inter-cluster distance table */
    void computeClusterDistances();

    /** @brief Single Lloyd iteration with triangle inequality */
    int lloydStep(const QVector<QVector<double>>& data);
};
