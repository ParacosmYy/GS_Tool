/**
 * @file SpectralCluster12.h
 * @brief 谱聚类(归一化割目标函数+k路离散化正交变换) — Spectral Clustering with Normalized Cut Objective and K-Way Discretization via Orthogonal Transformation
 *
 * 功能: 实现谱聚类(Spectral clustering)，采用归一化割(Normalized cut)目标函数构建
 *       归一化拉普拉斯矩阵(Normalized Laplacian)，通过k路离散化(k-way discretization)
 *       正交变换(orthogonal transformation)将连续谱嵌入映射为离散聚类标签。
 *
 * 协作: FuzzyCMeans11(模糊C均值) / KMeans23(K均值) / BirchClustering11(BIRCH聚类)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 谱聚类(归一化割目标函数+k路离散化正交变换)
 */
class SpectralCluster12 : public QObject {
    Q_OBJECT

public:
    /** @brief Cluster assignment result */
    struct Assignment {
        int pointIndex = -1;
        int cluster = -1;
        double confidence = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numPoints = 0;
        int numDimensions = 0;
        int numClusters = 0;
        int eigenIterations = 0;
        double ncutValue = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SpectralCluster12(QObject *parent = nullptr);
    ~SpectralCluster12() override;

    /** @brief Set number of clusters k */
    void setNumClusters(int k);

    /** @brief Set Gaussian kernel sigma for affinity */
    void setSigma(double sigma);

    /** @brief Set max power-iteration steps for eigensolver */
    void setMaxEigenIterations(int iter);

    /** @brief Set convergence tolerance for eigensolver */
    void setEigenTolerance(double tol);

    /** @brief Set k-nearest-neighbors for sparse affinity (0 = full) */
    void setKNN(int knn);

    /** @brief Run spectral clustering on [n x d] data */
    bool fit(const QVector<QVector<double>>& data);

    /** @brief Predict cluster for a new point */
    int predict(const QVector<double>& point) const;

    /** @brief Get cluster assignments */
    QVector<Assignment> assignments() const;

    /** @brief Get eigenvectors [n x k] */
    QVector<QVector<double>> eigenvectors() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void eigenSolved(int k, double residual);
    void fitCompleted(int clusters, double ncut, double timeMs);

private:
    int m_numClusters = 3;
    double m_sigma = 1.0;
    int m_maxEigenIter = 300;
    double m_eigenTol = 1e-6;
    int m_knn = 0;

    QVector<QVector<double>> m_data;
    QVector<QVector<double>> m_affinity;   // [n x n]
    QVector<QVector<double>> m_eigVecs;    // [n x k]
    QVector<Assignment> m_assignments;
    QVector<QVector<double>> m_centroids;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build Gaussian affinity matrix */
    void buildAffinity();

    /** @brief Build normalized Laplacian L = D^{-1/2} W D^{-1/2} */
    QVector<QVector<double>> buildNormalizedLaplacian() const;

    /** @brief Power iteration for top-k eigenvectors */
    bool solveTopKEigenvectors(const QVector<QVector<double>>& mat, int k);

    /** @brief QR orthogonalize columns of matrix */
    void orthogonalize(QVector<QVector<double>>& mat) const;

    /** @brief Row-normalize eigenvectors */
    QVector<QVector<double>> normalizeRows(const QVector<QVector<double>>& mat) const;

    /** @brief K-way discretization via rotation */
    void discretizeKWay();

    /** @brief Euclidean distance between two vectors */
    double euclideanDist(const QVector<double>& a, const QVector<double>& b) const;
};
