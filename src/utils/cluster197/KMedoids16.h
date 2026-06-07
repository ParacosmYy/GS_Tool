/**
 * @file KMedoids16.h
 * @brief K-中心点聚类(CLARANS随机交换+轮廓系数验证) — K-Medoids Clustering with CLARANS Randomized Swapping and Silhouette-based Cluster Validity
 *
 * 功能: 实现K-中心点聚类算法，支持CLARANS随机交换策略、
 *       轮廓系数聚类有效性评估和多种距离度量。
 *
 * 协作: GaussianMixture16(高斯混合) / KMeans18(K均值) / DBSCAN11(DBSCAN聚类)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief K-中心点聚类(CLARANS+轮廓系数)
 */
class KMedoids16 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalFits = 0;
        int numSamples = 0;
        int numClusters = 0;
        double silhouetteScore = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Distance metric type */
    enum class DistanceMetric { Euclidean, Manhattan, Cosine };

    explicit KMedoids16(QObject *parent = nullptr);
    ~KMedoids16() override;

    void setNumClusters(int k);
    void setMaxIterations(int iters);
    void setNumNeighbors(int neighbors);
    void setDistanceMetric(DistanceMetric metric);

    /** @brief Fit model to data, return cluster assignments */
    QVector<int> fit(const QVector<QVector<double>>& data);

    /** @brief Predict cluster for new samples */
    QVector<int> predict(const QVector<QVector<double>>& data) const;

    /** @brief Compute silhouette coefficient for current clustering */
    double computeSilhouette(const QVector<QVector<double>>& data,
                             const QVector<int>& labels) const;

    /** @brief Get medoid indices */
    QVector<int> medoids() const;

    /** @brief Compute pairwise distance matrix */
    QVector<QVector<double>> distanceMatrix(const QVector<QVector<double>>& data) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void fittingCompleted(int clusters, double silhouette, double timeMs);

private:
    int m_k = 3;
    int m_maxIter = 100;
    int m_numNeighbors = 10;
    DistanceMetric m_metric = DistanceMetric::Euclidean;

    QVector<int> m_medoidIndices;
    QVector<QVector<double>> m_distMatrix;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute distance between two points */
    double distance(const QVector<double>& a, const QVector<double>& b) const;

    /** @brief Assign each point to nearest medoid */
    QVector<int> assignClusters(const QVector<QVector<double>>& data) const;

    /** @brief Compute total cost (sum of distances to medoids) */
    double totalCost(const QVector<QVector<double>>& data,
                     const QVector<int>& medoids) const;

    /** @brief CLARANS: try random neighbor swaps */
    bool claransSwap(const QVector<QVector<double>>& data, int n);

    /** @brief Compute silhouette for a single point */
    double pointSilhouette(int idx, const QVector<int>& labels, int k) const;
};
