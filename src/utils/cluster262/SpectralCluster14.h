/**
 * @file SpectralCluster14.h
 * @brief 谱聚类(Shi-Malik归一化割特征向量二分图分割) — Spectral Clustering with Shi-Malik Normalized Cut and Eigenvector Bipartitioning for Graph Segmentation
 *
 * 功能: 实现谱聚类(Spectral clustering)，采用Shi-Malik归一化割(Shi-Malik normalized cut)
 *       和特征向量二分(eigenvector bipartitioning)实现图分割(graph segmentation)。
 *
 * 协作: FuzzyCMeans13(模糊C均值) / KMeans27(K均值) / DBSCAN15(密度聚类)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 谱聚类(Shi-Malik归一化割特征向量二分图分割)
 */
class SpectralCluster14 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numPoints = 0;
        int numClusters = 0;
        int numEigenUsed = 0;
        double ncutValue = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SpectralCluster14(QObject *parent = nullptr);
    ~SpectralCluster14() override;

    /** @brief Set number of clusters and sigma for RBF kernel */
    void setParameters(int numClusters, double sigma = 1.0, int maxIterations = 100);

    /** @brief Build affinity matrix from data using RBF kernel and run clustering */
    bool fit(const QVector<QVector<double>>& data);

    /** @brief Build from precomputed affinity matrix */
    bool fitFromAffinity(const QVector<QVector<double>>& affinity);

    /** @brief Get cluster labels (0..K-1) */
    QVector<int> labels() const;

    /** @brief Get eigenvectors used for embedding (N x K) */
    QVector<QVector<double>> eigenvectors() const;

    /** @brief Get normalized cut value */
    double ncutValue() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void clusteringUpdated(int numClusters, double ncutValue, double timeMs);

private:
    int m_K = 2;
    double m_sigma = 1.0;
    int m_maxIter = 100;
    int m_n = 0;

    QVector<QVector<double>> m_affinity;   // N x N affinity matrix
    QVector<QVector<double>> m_eigvecs;    // N x K eigenvectors
    QVector<int> m_labels;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build RBF kernel affinity matrix */
    void buildAffinity(const QVector<QVector<double>>& data);

    /** @brief Compute degree matrix and normalized Laplacian L = D^{-1/2}(D-W)D^{-1/2} */
    void computeNormalizedLaplacian(QVector<QVector<double>>& L) const;

    /** @brief Power iteration to find K smallest eigenvectors */
    void computeEigenvectors(const QVector<QVector<double>>& L, int k);

    /** @brief K-means on eigenvector rows to get final labels */
    void kMeansOnEmbedding();

    /** @brief Compute Ncut objective */
    double computeNcut() const;

    /** @brief Matrix-vector multiply */
    QVector<double> matVec(const QVector<QVector<double>>& M,
                           const QVector<double>& v) const;

    /** @brief Normalize vector to unit length */
    void normalizeVec(QVector<double>& v) const;

    /** @brief Dot product */
    static double dot(const QVector<double>& a, const QVector<double>& b);
};
