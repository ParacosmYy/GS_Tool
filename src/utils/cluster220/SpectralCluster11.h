/**
 * @file SpectralCluster11.h
 * @brief 谱聚类(未归一化拉普拉斯+特征间隙自动聚类数选择) — Spectral Clustering with Unnormalized Laplacian and Eigengap Heuristic for Automatic Cluster Count
 *
 * 功能: 实现谱聚类算法，基于未归一化拉普拉斯矩阵的特征分解，
 *       使用特征间隙启发式自动确定最优聚类数。
 *
 * 协作: FuzzyCMeans10(模糊聚类) / KMeans21(K均值) / DBSCAN12(密度聚类)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 谱聚类(未归一化拉普拉斯+特征间隙)
 */
class SpectralCluster11 : public QObject {
    Q_OBJECT

public:
    /** @brief Cluster result with labels and eigenvectors */
    struct ClusterResult {
        QVector<int> labels;
        QVector<QVector<double>> eigenvectors;
        QVector<double> eigenvalues;
        int numClusters = 0;
        double objective = 0.0;
        int iterations = 0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numPoints = 0;
        int dim = 0;
        int numClusters = 0;
        int maxClusters = 0;
        int totalIterations = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SpectralCluster11(QObject *parent = nullptr);
    ~SpectralCluster11() override;

    /** @brief Set parameters: max clusters, sigma for RBF kernel, max iterations */
    void setParameters(int maxClusters = 10, double sigma = 1.0, int maxIter = 100);

    /** @brief Fit data with automatic cluster count via eigengap */
    ClusterResult fit(const QVector<QVector<double>>& data);

    /** @brief Fit data with specified cluster count */
    ClusterResult fit(const QVector<QVector<double>>& data, int clusters);

    /** @brief Predict cluster labels for new data */
    QVector<int> predict(const QVector<QVector<double>>& data) const;

    /** @brief Get current centroids */
    QVector<QVector<double>> centroids() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void clusteringCompleted(int clusters, int iterations, double timeMs);

private:
    int m_maxClusters = 10;
    double m_sigma = 1.0;
    int m_maxIter = 100;
    int m_dim = 0;
    int m_clusters = 0;

    QVector<QVector<double>> m_centroids;
    QVector<QVector<double>> m_eigenvectors;
    QVector<double> m_eigenvalues;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute RBF similarity matrix */
    void computeSimilarity(const QVector<QVector<double>>& data,
                            QVector<QVector<double>>& W) const;

    /** @brief Build degree matrix from similarity */
    void computeDegree(const QVector<QVector<double>>& W,
                        QVector<double>& D) const;

    /** @brief Compute unnormalized Laplacian L = D - W */
    void computeLaplacian(const QVector<QVector<double>>& W,
                           const QVector<double>& D,
                           QVector<QVector<double>>& L) const;

    /** @brief QR-based eigenvalue decomposition (smallest eigenvalues) */
    void eigenDecompose(QVector<QVector<double>>& L,
                         int numEigenvectors);

    /** @brief Eigengap heuristic to select optimal cluster count */
    int eigengapHeuristic(const QVector<double>& evals) const;

    /** @brief K-means on eigenvector rows */
    QVector<int> kmeansOnEigenvectors(
        const QVector<QVector<double>>& evecs, int k) const;
};
