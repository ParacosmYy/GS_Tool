/**
 * @file KMeans27.h
 * @brief K均值二分聚类(SSE分裂准则层次分裂式聚类) — K-means with Bisecting Strategy and SSE-based Split Criterion for Hierarchical Divisive Clustering
 *
 * 功能: 实现K均值二分聚类(bisecting K-means)，采用SSE分裂准则
 *       (SSE-based split criterion)进行层次分裂式聚类(hierarchical
 *       divisive clustering)，适合大规模数据的高效分层聚类。
 *
 * 协作: GaussianMixture28(高斯混合) / DBSCAN15(密度聚类) / KMeans26(核K均值)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief K均值二分聚类(SSE分裂准则层次分裂式聚类)
 */
class KMeans27 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numClusters = 0;
        int numSamples = 0;
        int numDimensions = 0;
        int numSplits = 0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Single cluster with centroid */
    struct Cluster {
        QVector<double> centroid;
        QVector<int> memberIndices;
        double sse = 0.0;
    };

    explicit KMeans27(QObject *parent = nullptr);
    ~KMeans27() override;

    /** @brief Set target number of clusters */
    void setNumClusters(int k);

    /** @brief Set max iterations per bisect step */
    void setMaxIterations(int iters);

    /** @brief Run bisecting K-means on data */
    bool fit(const QVector<QVector<double>>& data);

    /** @brief Predict cluster label for a sample */
    int predict(const QVector<double>& sample) const;

    /** @brief Get all clusters */
    QVector<Cluster> clusters() const;

    /** @brief Get total SSE across all clusters */
    double totalSSE() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void clusteringCompleted(int numClusters, double totalSSE, double timeMs);
    void clusterSplit(int clusterId, double sseBefore, double sseAfter);

private:
    int m_k = 8;
    int m_maxIter = 50;
    int m_dims = 0;

    QVector<Cluster> m_clusters;
    QVector<QVector<double>> m_data;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute SSE of a subset of data points */
    double computeSSE(const QVector<int>& indices, const QVector<double>& centroid) const;

    /** @brief Basic K-means on a subset, returns 2 clusters */
    QVector<Cluster> bisect(const QVector<int>& indices) const;

    /** @brief Find the cluster with highest SSE */
    int findMaxSSECluster() const;

    /** @brief Compute Euclidean distance squared */
    double distSq(const QVector<double>& a, const QVector<double>& b) const;

    /** @brief Compute centroid of a subset */
    QVector<double> computeCentroid(const QVector<int>& indices) const;
};
