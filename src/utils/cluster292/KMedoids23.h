/**
 * @file KMedoids23.h
 * @brief K-中心点聚类(CLARA采样与轮廓系数引导中心点交换实现大规模鲁棒PAM聚类) — K-medoids with CLARA Sampling and Silhouette-guided Medoid Swap for Large-scale Robust Partitioning Around Medoids
 *
 * 功能: 实现K-中心点聚类(K-medoids)，采用CLARA采样(CLARA sampling)
 *       与轮廓系数引导中心点交换(silhouette-guided medoid swap)实现大规模鲁棒PAM聚类(large-scale robust PAM clustering)。
 *
 * 协作: GaussianMixture35(高斯混合模型) / KMeans31(K-means) / BirchClustering15(BIRCH聚类)
 */
#pragma once

#include <QObject>
#include <QVector>

class KMedoids23 : public QObject {
    Q_OBJECT

public:
    /** @brief Clustering result */
    struct ClusterResult {
        QVector<int> medoidIndices;
        QVector<int> assignments;
        QVector<double> distances;
        double silhouetteScore = 0.0;
        double totalCost = 0.0;
        int numIterations = 0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalFits = 0;
        int numClusters = 0;
        int numPoints = 0;
        double avgSilhouette = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit KMedoids23(QObject *parent = nullptr);
    ~KMedoids23() override;

    void setNumClusters(int k);
    void setMaxIterations(int maxIter);
    void setClaraSamples(int samples);
    void setSampleSize(int size);

    /** @brief Fit using full PAM on small data or CLARA on large data */
    ClusterResult fit(const QVector<QVector<double>>& data);

    /** @brief Compute pairwise distance matrix (Euclidean) */
    QVector<QVector<double>> computeDistanceMatrix(
        const QVector<QVector<double>>& data) const;

    /** @brief Predict cluster for new points */
    QVector<int> predict(const QVector<QVector<double>>& data,
                         const QVector<QVector<double>>& medoids) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void fitDone(int n, int k, double silhouette, double timeMs);

private:
    int m_k = 3;
    int m_maxIter = 100;
    int m_claraSamples = 5;
    int m_sampleSize = 256;
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_silSum = 0.0;

    /** @brief Run PAM on a subset */
    ClusterResult pamCore(const QVector<QVector<double>>& distMatrix,
                           const QVector<int>& indices) const;

    /** @brief Assign each point to nearest medoid */
    QVector<int> assignToMedoids(const QVector<QVector<double>>& distMatrix,
                                  const QVector<int>& medoids) const;

    /** @brief Compute total cost (sum of distances to medoid) */
    double computeTotalCost(const QVector<QVector<double>>& distMatrix,
                             const QVector<int>& medoids,
                             const QVector<int>& assignments) const;

    /** @brief Compute silhouette score */
    double computeSilhouette(const QVector<QVector<double>>& distMatrix,
                              const QVector<int>& assignments, int k) const;

    /** @brief Euclidean distance between two vectors */
    double euclideanDistance(const QVector<double>& a,
                             const QVector<double>& b) const;
};
