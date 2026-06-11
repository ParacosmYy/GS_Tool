/**
 * @file KMeans32.h
 * @brief K均值聚类(小批量随机梯度与余弦相似度实现球面高维空间聚类分配) — K-Means with Mini-Batch Stochastic Gradient and Cosine Similarity Metric for Spherical Cluster Assignment in High-Dimensional Spaces
 *
 * 功能: 实现K均值聚类(K-means clustering)，采用小批量随机梯度(mini-batch stochastic gradient)
 *       与余弦相似度度量(cosine similarity metric)实现球面高维空间聚类分配(spherical cluster assignment in high-dimensional spaces)。
 *
 * 协作: GaussianMixture36(高斯混合模型) / DBSCAN(密度聚类) / OPTICS14(有序聚类)
 */
#pragma once

#include <QObject>
#include <QVector>

class KMeans32 : public QObject {
    Q_OBJECT

public:
    /** @brief Cluster centroid with associated statistics */
    struct Centroid {
        QVector<double> coordinates;
        int assignmentCount = 0;
        double inertia = 0.0;
    };

    /** @brief Clustering result */
    struct ClusterResult {
        QVector<int> assignments;
        QVector<Centroid> centroids;
        double totalInertia = 0.0;
        int iterations = 0;
        bool converged = false;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalFits = 0;
        int numClusters = 0;
        int dimensions = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit KMeans32(QObject *parent = nullptr);
    ~KMeans32() override;

    void setNumClusters(int k);
    void setMaxIterations(int maxIter);
    void setBatchSize(int size);
    void setConvergenceTolerance(double tol);

    /** @brief Fit K-means to multi-dimensional data using mini-batch SGD with cosine similarity */
    ClusterResult fit(const QVector<QVector<double>>& data);

    /** @brief Assign new points to nearest centroid via cosine similarity */
    QVector<int> predict(const QVector<QVector<double>>& points) const;

    /** @brief Get current centroids */
    const QVector<Centroid>& centroids() const { return m_centroids; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void fitDone(int k, int iters, double inertia, double timeMs);

private:
    int m_k = 8;
    int m_maxIter = 200;
    int m_batchSize = 256;
    double m_tol = 1e-6;
    int m_dims = 0;
    QVector<Centroid> m_centroids;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Cosine similarity between two vectors */
    double cosineSimilarity(const QVector<double>& a, const QVector<double>& b) const;

    /** @brief L2 normalize a vector in place */
    void normalize(QVector<double>& v) const;

    /** @brief Initialize centroids via k-means++ on normalized vectors */
    void initializeCentroids(const QVector<QVector<double>>& data);

    /** @brief Select mini-batch indices deterministically */
    QVector<int> selectBatch(int totalSize, int batchSize, int iteration) const;

    /** @brief Find nearest centroid by cosine similarity */
    int nearestCentroid(const QVector<double>& point) const;

    /** @brief Compute total inertia (sum of 1 - cosine similarity) */
    double computeInertia(const QVector<QVector<double>>& data,
                          const QVector<int>& assignments) const;
};
