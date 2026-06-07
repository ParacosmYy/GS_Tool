/**
 * @file SpectralCluster9.h
 * @brief 谱聚类(Fiedler向量二分+多类递归分裂) — Spectral Clustering with Bi-Partitioning via Fiedler Vector and Multi-Class Recursive Splitting
 *
 * 功能: 实现谱聚类算法，支持相似度图构建、Laplacian矩阵分解、
 *       Fiedler向量二分和递归多类分裂。
 *
 * 协作: OPTICS7(OPTICS聚类) / KMeans5(K均值) / SubspaceCluster6(子空间聚类)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 谱聚类(Fiedler向量二分+多类递归分裂)
 */
class SpectralCluster9 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalFits = 0;
        int numSamples = 0;
        int numClusters = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SpectralCluster9(QObject *parent = nullptr);
    ~SpectralCluster9() override;

    void setNumClusters(int k);
    void setSigma(double sigma);
    void setKNeighbors(int k);

    /** @brief Run spectral clustering on n x d data matrix */
    QVector<int> fit(const QVector<QVector<double>>& data);

    /** @brief Build affinity matrix using RBF kernel */
    QVector<QVector<double>> buildAffinity(const QVector<QVector<double>>& data) const;

    /** @brief Compute normalized Laplacian from affinity matrix */
    void computeLaplacian(const QVector<QVector<double>>& affinity,
                           QVector<QVector<double>>& laplacian) const;

    /** @brief Compute Fiedler vector (2nd smallest eigenvector) via power iteration */
    QVector<double> fiedlerVector(const QVector<QVector<double>>& laplacian) const;

    /** @brief Bi-partition points by Fiedler vector sign */
    QPair<QVector<int>, QVector<int>> biPartition(const QVector<double>& fiedler) const;

    /** @brief Recursive splitting to reach target number of clusters */
    QVector<int> recursiveSplit(const QVector<QVector<double>>& data,
                                 const QVector<int>& indices, int targetK) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void clusteringCompleted(int clusters, double timeMs);

private:
    int m_k = 2;
    double m_sigma = 1.0;
    int m_kNeighbors = 10;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Euclidean distance between two points */
    static double distance(const QVector<double>& a, const QVector<double>& b);

    /** @brief Power iteration for dominant eigenvector */
    static QVector<double> powerIteration(const QVector<QVector<double>>& mat, int maxIter);

    /** @brief Normalize a vector to unit length */
    static void normalizeVector(QVector<double>& v);
};
