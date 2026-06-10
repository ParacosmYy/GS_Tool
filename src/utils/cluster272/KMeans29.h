/**
 * @file KMeans29.h
 * @brief K均值聚类(小批量随机梯度与核心集构造大规模数据可扩展聚类) — K-means with Mini-batch Stochastic Gradient and Coreset Construction for Scalable Large-dataset Clustering
 *
 * 功能: 实现K均值聚类(K-means)，采用小批量随机梯度(mini-batch stochastic gradient)
 *       与核心集构造(coreset construction)实现大规模数据可扩展聚类(scalable clustering)。
 *
 * 协作: GaussianMixture31(高斯混合) / DBSCAN16(密度聚类) / OPTICS12(排序聚类)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief K均值聚类(小批量随机梯度与核心集构造)
 */
class KMeans29 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numClusters = 0;
        int numSamples = 0;
        int numIterations = 0;
        double inertia = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit KMeans29(QObject *parent = nullptr);
    ~KMeans29() override;

    /** @brief Set number of clusters K */
    void setClusters(int k);

    /** @brief Set maximum iterations */
    void setMaxIterations(int iters);

    /** @brief Set mini-batch size for stochastic updates */
    void setBatchSize(int size);

    /** @brief Set convergence tolerance */
    void setTolerance(double tol);

    /** @brief Build coreset from data for scalable initialization */
    void buildCoreset(const QVector<QVector<double>>& data, int coresetSize);

    /** @brief Fit model: returns cluster assignments per point */
    QVector<int> fit(const QVector<QVector<double>>& data);

    /** @brief Mini-batch update step */
    void miniBatchUpdate(const QVector<QVector<double>>& batch);

    /** @brief Predict cluster for a single point */
    int predict(const QVector<double>& point) const;

    /** @brief Get cluster centroids */
    QVector<QVector<double>> centroids() const;

    /** @brief Get inertia (sum of squared distances to centroids) */
    double inertia() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void fittingDone(int iterations, double inertia, double timeMs);

private:
    int m_k = 8;
    int m_maxIter = 200;
    int m_batchSize = 256;
    double m_tol = 1e-4;

    QVector<QVector<double>> m_centroids;
    QVector<int> m_counts;          // Per-centroid assignment counts
    QVector<QVector<double>> m_coreset;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief K-means++ initialization */
    void initCentroids(const QVector<QVector<double>>& data);

    /** @brief Nearest centroid index and squared distance */
    int nearestCentroid(const QVector<double>& point, double& distSq) const;

    /** @brief Compute total inertia */
    double computeInertia(const QVector<QVector<double>>& data,
                          const QVector<int>& labels) const;

    /** @brief Sample a mini-batch from data */
    QVector<int> sampleBatch(int n) const;
};
