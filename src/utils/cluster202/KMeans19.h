/**
 * @file KMeans19.h
 * @brief K均值聚类(流式Mini-Batch+蓄水池采样+质心动量更新) — K-Means with Streaming Mini-Batch via Reservoir Sampling and Centroid Momentum Update
 *
 * 功能: 实现流式Mini-Batch K-means聚类，支持蓄水池采样、
 *       质心动量更新和在线增量学习。
 *
 * 协作: SpectralCluster9(谱聚类) / OPTICS7(OPTICS聚类) / DBSCAN8(密度聚类)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief K均值聚类(流式Mini-Batch+蓄水池采样+质心动量更新)
 */
class KMeans19 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalFits = 0;
        int numSamples = 0;
        int numClusters = 0;
        double inertia = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit KMeans19(QObject *parent = nullptr);
    ~KMeans19() override;

    void setNumClusters(int k);
    void setBatchSize(int size);
    void setMaxIterations(int iter);
    void setMomentum(double beta);

    /** @brief Batch fit: initialize centroids and run full K-means */
    QVector<int> fit(const QVector<QVector<double>>& data);

    /** @brief Stream a mini-batch through reservoir sampling */
    void partialFit(const QVector<QVector<double>>& batch);

    /** @brief Reservoir sample a mini-batch from stream buffer */
    QVector<QVector<double>> reservoirSample(int batchSize) const;

    /** @brief Update centroids with momentum on sampled batch */
    void momentumUpdate(const QVector<QVector<double>>& batch, const QVector<int>& assignments);

    /** @brief Assign each point to nearest centroid */
    QVector<int> predict(const QVector<QVector<double>>& data) const;

    /** @brief Get current centroids */
    QVector<QVector<double>> centroids() const;

    /** @brief Compute inertia (sum of squared distances to assigned centroids) */
    double computeInertia(const QVector<QVector<double>>& data) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void clusteringCompleted(int clusters, double inertia, double timeMs);

private:
    int m_k = 8;
    int m_batchSize = 100;
    int m_maxIter = 300;
    double m_momentum = 0.9;

    QVector<QVector<double>> m_centroids;
    QVector<QVector<double>> m_velocity;     // momentum velocity for each centroid
    QVector<QVector<double>> m_streamBuffer; // reservoir of past samples
    QVector<int> m_clusterCounts;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Euclidean distance between two points */
    static double distance(const QVector<double>& a, const QVector<double>& b);

    /** @brief Initialize centroids using K-means++ seeding */
    void kmeansPlusPlusInit(const QVector<QVector<double>>& data);

    /** @brief Find nearest centroid for a single point */
    int nearestCentroid(const QVector<double>& point) const;
};
