/**
 * @file KMeans25.h
 * @brief K-means聚类(小批量随机梯度下降+逐迭代收敛跟踪的大规模聚类) — K-means with Mini-Batch Stochastic Gradient and Per-Iteration Convergence Tracking for Large-Scale Clustering
 *
 * 功能: 实现K-means聚类(K-means Clustering)，使用小批量随机梯度下降
 *       (mini-batch stochastic gradient descent)进行高效质心更新，通过
 *       逐迭代收敛跟踪(per-iteration convergence tracking)监控聚类质量。
 *
 * 协作: GaussianMixture25(高斯混合模型) / DBSCAN14(密度聚类) / OPTICS10(排序聚类)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief K-means聚类(小批量随机梯度下降+逐迭代收敛跟踪)
 */
class KMeans25 : public QObject {
    Q_OBJECT

public:
    /** @brief Convergence info per iteration */
    struct IterationRecord {
        double inertia = 0.0;
        double centroidShift = 0.0;
        int iteration = 0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numClusters = 0;
        int numSamples = 0;
        int numDimensions = 0;
        int totalIterations = 0;
        double finalInertia = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit KMeans25(QObject *parent = nullptr);
    ~KMeans25() override;

    /** @brief Set number of clusters */
    void setK(int k);

    /** @brief Set mini-batch size */
    void setBatchSize(int size);

    /** @brief Set convergence threshold (centroid shift) */
    void setTolerance(double tol);

    /** @brief Set maximum iterations */
    void setMaxIterations(int iters);

    /** @brief Fit model to data using mini-batch K-means */
    void fit(const QVector<QVector<double>>& data);

    /** @brief Predict cluster for a single point */
    int predict(const QVector<double>& point) const;

    /** @brief Get cluster assignments */
    QVector<int> labels() const;

    /** @brief Get cluster centroids */
    QVector<QVector<double>> centroids() const;

    /** @brief Get per-iteration convergence history */
    QVector<IterationRecord> convergenceHistory() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void fittingCompleted(int k, double inertia, int iterations, double timeMs);

private:
    int m_k = 8;
    int m_batchSize = 256;
    double m_tol = 1e-4;
    int m_maxIter = 300;

    QVector<QVector<double>> m_centroids;
    QVector<int> m_labels;
    QVector<int> m_counts;       // Per-centroid assignment counts
    QVector<IterationRecord> m_history;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Initialize centroids via k-means++ seeding */
    void initCentroids(const QVector<QVector<double>>& data);

    /** @brief Find nearest centroid for a point */
    int nearestCentroid(const QVector<double>& point) const;

    /** @brief Compute squared Euclidean distance */
    double sqDistance(const QVector<double>& a, const QVector<double>& b) const;

    /** @brief Compute total inertia (sum of squared distances) */
    double computeInertia(const QVector<QVector<double>>& data) const;
};
