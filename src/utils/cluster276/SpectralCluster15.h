/**
 * @file SpectralCluster15.h
 * @brief 谱聚类(Ng-Jordan-Weiss归一化谱嵌入与旋转自动k确定的流形学习聚类) — Spectral Clustering with Ng-Jordan-Weiss Normalized Spectral Embedding and Rotation-based Auto-k Determination
 *
 * 功能: 实现谱聚类(Spectral clustering)，采用Ng-Jordan-Weiss归一化谱嵌入(NJW normalized spectral embedding)
 *       与旋转自动k确定(rotation-based auto-k)实现流形学习聚类(manifold learning clustering)。
 *
 * 协作: FuzzyCMeans14(模糊C均值) / KMeans29(K均值) / DBSCAN16(密度聚类)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 谱聚类(Ng-Jordan-Weiss归一化谱嵌入与旋转自动k确定)
 */
class SpectralCluster15 : public QObject {
    Q_OBJECT

public:
    /** @brief Clustering result */
    struct ClusterResult {
        QVector<QVector<double>> centroids;
        QVector<int> labels;
        QVector<QVector<double>> eigenvectors;  // Top-k eigenvectors
        QVector<double> eigenvalues;             // Top-k eigenvalues
        int optimalK = 0;
        double objectiveValue = 0.0;
        int iterations = 0;
        bool converged = false;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numPoints = 0;
        int numClusters = 0;
        double sigma = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SpectralCluster15(QObject *parent = nullptr);
    ~SpectralCluster15() override;

    /** @brief Set RBF kernel bandwidth sigma */
    void setSigma(double sigma);

    /** @brief Set number of neighbors for affinity graph */
    void setKNeighbors(int k);

    /** @brief Set max clusters for auto-k search */
    void setMaxClusters(int k);

    /** @brief Set convergence tolerance */
    void setTolerance(double tol);

    /** @brief Run spectral clustering with auto-k or fixed k */
    ClusterResult fit(const QVector<QVector<double>>& data, int k = 0);

    /** @brief Determine optimal k via eigengap heuristic */
    int determineOptimalK(const QVector<double>& eigenvalues) const;

    /** @brief Compute normalized cut value for validation */
    double normalizedCut(const QVector<QVector<double>>& data,
                          const QVector<int>& labels, int k) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void clusteringDone(int k, double objective, double timeMs);
    void eigenvalueComputed(int idx, double value);

private:
    double m_sigma = 1.0;
    int m_kNeighbors = 7;
    int m_maxClusters = 10;
    double m_tolerance = 1e-6;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build affinity matrix using RBF kernel */
    QVector<QVector<double>> buildAffinityMatrix(const QVector<QVector<double>>& data) const;

    /** @brief Build normalized graph Laplacian (NJW) */
    void buildNJWLaplacian(const QVector<QVector<double>>& W,
                            QVector<QVector<double>>& L) const;

    /** @brief Compute top-k eigenvectors via power iteration */
    void computeEigenvectors(const QVector<QVector<double>>& L, int k,
                              QVector<QVector<double>>& eigvecs,
                              QVector<double>& eigvals);

    /** @brief Normalize rows of eigenvector matrix */
    void normalizeRows(QVector<QVector<double>>& mat) const;

    /** @brief K-means on spectral embedding */
    QVector<int> kmeansOnEmbedding(const QVector<QVector<double>>& embedded, int k);

    /** @brief Euclidean distance squared */
    double distSq(const QVector<double>& a, const QVector<double>& b) const;
};
