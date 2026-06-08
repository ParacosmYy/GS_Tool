/**
 * @file KMeans22.h
 * @brief K均值(小批量随机梯度+余弦退火自适应学习率) — K-Means with Mini-batch Stochastic Gradient and Adaptive Learning Rate with Cosine Annealing Schedule
 *
 * 功能: 实现K均值聚类算法，采用小批量随机梯度(mini-batch SGD)更新质心，
 *       配合余弦退火(cosine annealing)自适应学习率调度，加速收敛并避免局部最优。
 *
 * 协作: GaussianMixture21(高斯混合) / KMedoids18(K中心点) / DBSCAN10(密度聚类)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief K均值(小批量SGD+余弦退火学习率)
 */
class KMeans22 : public QObject {
    Q_OBJECT

public:
    /** @brief Single cluster centroid */
    struct Centroid {
        QVector<double> coords;
        int count = 0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numClusters = 0;
        int numDimensions = 0;
        int numPoints = 0;
        int iterations = 0;
        double inertia = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit KMeans22(QObject *parent = nullptr);
    ~KMeans22() override;

    /** @brief Set cluster count and mini-batch size */
    void setParameters(int numClusters, int batchSize, int maxIter = 200);

    /** @brief Fit K-means to 2-D data (each inner vector is one point) */
    bool fit(const QVector<QVector<double>>& data);

    /** @brief Assign a point to its nearest cluster */
    int predict(const QVector<double>& point) const;

    /** @brief Get cluster centroids */
    QVector<Centroid> centroids() const;

    /** @brief Compute total within-cluster sum of squares */
    double inertia() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void fitCompleted(int iters, double inertia, double timeMs);
    void iterationProgress(int iter, double learningRate);

private:
    int m_numClusters = 3;
    int m_batchSize = 32;
    int m_maxIter = 200;
    int m_dims = 0;
    int m_seed = 42;

    QVector<Centroid> m_centroids;
    QVector<QVector<double>> m_data;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Squared Euclidean distance */
    double squaredDist(const QVector<double>& a,
                       const QVector<double>& b) const;

    /** @brief Find nearest centroid index */
    int nearestCentroid(const QVector<double>& point) const;

    /** @brief Cosine annealing learning rate for current iteration */
    double cosineAnnealing(int iter) const;

    /** @brief Initialize centroids via K-means++ seeding */
    void kmeansPPInit();

    /** @brief LCG random number generator */
    double randUniform();
};

