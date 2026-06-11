/**
 * @file KMeans30.h
 * @brief K-means聚类(Hartigan-Wong直接更新与簇内平方和最小化) — K-means with Hartigan-Wong Direct Update and Within-cluster Sum of Squares Minimization for Efficient Centroid Relocation
 *
 * 功能: 实现K-means聚类算法，采用Hartigan-Wong直接更新(Hartigan-Wong direct update)
 *       与簇内平方和最小化(WCSS minimization)实现高效质心重定位(centroid relocation)。
 *
 * 协作: GaussianMixture33(高斯混合) / OPTICS13(密度聚类) / DBSCAN14(密度聚类)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief K-means聚类(Hartigan-Wong直接更新)
 */
class KMeans30 : public QObject {
    Q_OBJECT

public:
    /** @brief Cluster result */
    struct ClusterResult {
        QVector<QVector<double>> centroids;
        QVector<int> labels;
        QVector<double> wcss;         // Within-cluster sum of squares per cluster
        double totalWcss = 0.0;
        int iterations = 0;
        bool converged = false;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numClusters = 0;
        int numPoints = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit KMeans30(QObject *parent = nullptr);
    ~KMeans30() override;

    void setNumClusters(int k);
    void setMaxIterations(int iters);
    void setTolerance(double tol);
    void setSeed(unsigned int seed);

    /** @brief Fit K-means with Hartigan-Wong algorithm */
    ClusterResult fit(const QVector<QVector<double>>& data);

    /** @brief Predict nearest cluster for new samples */
    QVector<int> predict(const QVector<QVector<double>>& samples) const;

    /** @brief Compute WCSS for given data and current centroids */
    double computeWcss(const QVector<QVector<double>>& data) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void fitDone(int n, int k, double wcss, double timeMs);

private:
    int m_k = 3;
    int m_maxIter = 300;
    double m_tol = 1e-6;
    unsigned int m_seed = 42;
    int m_dim = 0;
    Stats m_stats;
    double m_timeSum = 0.0;

    QVector<QVector<double>> m_centroids;
    QVector<int> m_labels;
    QVector<double> m_clusterWcss;
    QVector<int> m_clusterSizes;

    /** @brief K-means++ initialization */
    void initCentroids(const QVector<QVector<double>>& data);

    /** @brief Compute squared Euclidean distance */
    double sqDist(const QVector<double>& a, const QVector<double>& b) const;

    /** @brief Hartigan-Wong single-point transfer */
    bool hartiganWongStep(const QVector<QVector<double>>& data);

    /** @brief Update all centroids from current assignments */
    void updateCentroids(const QVector<QVector<double>>& data);

    /** @brief Recompute WCSS per cluster */
    void recomputeWcss(const QVector<QVector<double>>& data);
};
