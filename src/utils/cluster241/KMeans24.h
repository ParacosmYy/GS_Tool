/**
 * @file KMeans24.h
 * @brief K-means聚类(Elkan三角不等式加速+Hamerly下界减少距离计算) — K-means Clustering with Elkan Triangle Inequality Acceleration and Hamerly Lower Bound for Reduced Distance Computations
 *
 * 功能: 实现K-means聚类算法，利用Elkan三角不等式(Elkan triangle inequality)
 *       维护点-中心距离上界/下界以跳过冗余距离计算，结合Hamerly单下界
 *       (Hamerly lower bound)进一步降低每轮迭代计算量。
 *
 * 协作: GaussianMixture24(高斯混合) / KMedoids19(K-中心点) / OPTICS10(密度聚类)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief K-means聚类(Elkan三角不等式加速+Hamerly下界减少距离计算)
 */
class KMeans24 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numClusters = 0;
        int numSamples = 0;
        int numDimensions = 0;
        int iterationsUsed = 0;
        quint64 distancesComputed = 0;
        quint64 distancesSkipped = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit KMeans24(QObject *parent = nullptr);
    ~KMeans24() override;

    /** @brief Set number of clusters k */
    void setNumClusters(int k);

    /** @brief Set maximum iterations */
    void setMaxIterations(int iters);

    /** @brief Set convergence threshold for centroid movement */
    void setConvergenceThreshold(double tol);

    /** @brief Run clustering, returns centroid vectors */
    QVector<QVector<double>> fit(const QVector<QVector<double>>& data);

    /** @brief Get cluster assignment for each sample */
    QVector<int> labels() const;

    /** @brief Get centroid vectors */
    QVector<QVector<double>> centroids() const;

    /** @brief Compute total inertia (sum of squared distances) */
    double inertia() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void iterationCompleted(int iter, double inertia, quint64 distComputed);

private:
    int m_k = 3;
    int m_maxIter = 100;
    double m_tol = 1e-6;

    QVector<QVector<double>> m_data;
    QVector<QVector<double>> m_centroids;
    QVector<int> m_labels;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Squared Euclidean distance between two vectors */
    double sqDist(const QVector<double>& a, const QVector<double>& b) const;

    /** @brief K-means++ initialization for centroids */
    void initCentroids();

    /** @brief Compute inter-centroid distances and return half-distance matrix */
    QVector<QVector<double>> centroidHalfDistances() const;

    /** @brief One iteration with Elkan bounds */
    bool elkanStep(QVector<double>& upper, QVector<QVector<double>>& lower,
                   QVector<double>& hamerlyBound);

    /** @brief Update centroids from current assignments */
    void updateCentroids();
};
