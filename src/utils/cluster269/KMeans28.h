/**
 * @file KMeans28.h
 * @brief K均值聚类(Elkan三角不等式加速与上下界剪枝距离计算优化) — K-Means with Elkan's Triangle Inequality Acceleration and Lower/Upper Bound Pruning for Distance Computation Reduction
 *
 * 功能: 实现K均值聚类(K-means clustering)，采用Elkan三角不等式加速(Elkan's triangle inequality acceleration)
 *       与上下界剪枝(lower/upper bound pruning)实现距离计算优化(distance computation reduction)。
 *
 * 协作: GaussianMixture30(高斯混合) / KMedoids21(K-中心点) / OPTICS12(密度聚类)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief K均值聚类(Elkan三角不等式加速与上下界剪枝距离计算优化)
 */
class KMeans28 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numPoints = 0;
        int numClusters = 0;
        int iterations = 0;
        double inertia = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit KMeans28(QObject *parent = nullptr);
    ~KMeans28() override;

    /** @brief Set number of clusters K */
    void setK(int k);

    /** @brief Set maximum iterations */
    void setMaxIterations(int iters);

    /** @brief Set convergence tolerance for centroid shift */
    void setTolerance(double tol);

    /** @brief Fit model to 2D data, returns centroids */
    QVector<QVector<double>> fit(const QVector<QVector<double>>& data);

    /** @brief Predict nearest cluster for each point */
    QVector<int> predict(const QVector<QVector<double>>& data) const;

    /** @brief Get cluster assignments from last fit */
    QVector<int> labels() const;

    /** @brief Get centroids */
    QVector<QVector<double>> centroids() const;

    /** @brief Get sum of squared distances to nearest centroid */
    double inertia() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void clusteringDone(int k, int iters, double inertia, double timeMs);

private:
    int m_k = 8;
    int m_maxIter = 300;
    double m_tol = 1e-4;

    QVector<QVector<double>> m_centroids;
    QVector<int> m_labels;
    double m_inertia = 0.0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Euclidean distance squared between two vectors */
    double distSq(const QVector<double>& a, const QVector<double>& b) const;

    /** @brief Initialize centroids via k-means++ seeding */
    void initCentroids(const QVector<QVector<double>>& data);

    /** @brief Elkan's accelerated iteration: uses bounds to skip distance computations */
    int elkanIterate(const QVector<QVector<double>>& data,
                     QVector<QVector<double>>& lowerBounds,
                     QVector<double>& upperBounds,
                     QVector<double>& sCache);

    /** @brief Compute half inter-centroid distances: s[j] = 0.5 * min_{k!=j} d(c_j, c_k) */
    QVector<double> computeHalfInterCentroid() const;
};
