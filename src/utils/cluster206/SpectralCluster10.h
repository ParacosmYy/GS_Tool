/**
 * @file SpectralCluster10.h
 * @brief 谱聚类(归一化割+特征向量旋转自动K选择) — Spectral Clustering with Normalized Cut and Eigenvector Rotation-Based Automatic K Selection
 *
 * 功能: 实现谱聚类算法，支持归一化拉普拉斯矩阵、
 *       特征向量旋转启发式自动K选择和归一化割优化。
 *
 * 协作: FuzzyCMeans9(模糊C均值) / KMeans19(K均值) / BirchClustering9(BIRCH增量聚类)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 谱聚类(归一化割+特征向量旋转自动K选择)
 */
class SpectralCluster10 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numPoints = 0;
        int numClusters = 0;
        int eigenIterations = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SpectralCluster10(QObject *parent = nullptr);
    ~SpectralCluster10() override;

    void setMaxClusters(int maxK);
    void setSigma(double sigma);
    void setMaxIterations(int maxIter);

    /** @brief Build normalized affinity matrix from data */
    void buildAffinityMatrix(const QVector<QVector<double>>& data);

    /** @brief Compute normalized Laplacian eigenvectors */
    void computeEigenvectors(int numEigens);

    /** @brief Automatic K selection via eigenvector rotation heuristic */
    int autoSelectK() const;

    /** @brief Run spectral clustering with given or auto K */
    void fit(const QVector<QVector<double>>& data, int k = 0);

    /** @brief Get cluster assignments */
    QVector<int> getLabels() const;

    /** @brief Get eigenvectors used for clustering */
    QVector<QVector<double>> getEigenvectors() const;

    /** @brief Compute normalized cut value for current partition */
    double computeNcut() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void clusteringCompleted(int clusters, int eigenIters, double timeMs);

private:
    int m_maxK = 10;
    double m_sigma = 1.0;
    int m_maxIter = 200;
    int m_n = 0;
    int m_dim = 0;

    QVector<QVector<double>> m_affinity;    // W matrix
    QVector<QVector<double>> m_eigenvectors; // Top-k eigenvectors
    QVector<double> m_eigenvalues;
    QVector<int> m_labels;
    QVector<QVector<double>> m_data;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Gaussian kernel affinity */
    double gaussianAffinity(const QVector<double>& a,
                             const QVector<double>& b) const;

    /** @brief Symmetric normalize Laplacian: D^{-1/2} W D^{-1/2} */
    void normalizeLaplacian();

    /** @brief Power iteration for top eigenvectors */
    void powerIteration(int numEigens);

    /** @brief K-means on eigenvector rows */
    void kMeansOnEigen(int k);

    /** @brief Rotation cost for auto-K heuristic */
    double rotationCost(int k) const;
};
