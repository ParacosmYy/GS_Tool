/**
 * @file SpectralCluster16.h
 * @brief 谱聚类(归一化割比率与k路离散化特征映射嵌入实现多类图划分) — Spectral Clustering with Normalized Cut Ratio and K-way Discretization via Eigenmap Embedding for Multi-class Graph Partitioning
 *
 * 功能: 实现谱聚类(Spectral clustering)，采用归一化割比率(normalized cut ratio)
 *       与k路离散化(k-way discretization)通过特征映射嵌入(eigenmap embedding)实现多类图划分(multi-class graph partitioning)。
 *
 * 协作: FuzzyCMeans15(模糊C均值) / KMeans31(K-means) / BirchClustering15(BIRCH聚类)
 */
#pragma once

#include <QObject>
#include <QVector>

class SpectralCluster16 : public QObject {
    Q_OBJECT

public:
    /** @brief Clustering result from spectral embedding */
    struct ClusterResult {
        QVector<int> labels;               // Cluster assignment per point
        QVector<QVector<double>> embeddings; // Spectral embedding [n×k]
        double ncutValue = 0.0;            // Normalized cut objective
        int numClusters = 0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numPoints = 0;
        int numClusters = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SpectralCluster16(QObject *parent = nullptr);
    ~SpectralCluster16() override;

    void setNumClusters(int k);
    void setSigma(double sigma);           // RBF kernel bandwidth
    void setMaxIterations(int maxIter);
    void setKnnNeighbors(int knn);         // kNN graph connectivity

    /** @brief Build similarity graph and perform spectral clustering */
    ClusterResult fit(const QVector<QVector<double>>& data);

    /** @brief Compute normalized cut ratio for given partition */
    double normalizedCutRatio(const QVector<QVector<double>>& similarity,
                               const QVector<int>& labels) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void fitDone(int n, int k, double ncut, double timeMs);

private:
    int m_k = 3;
    double m_sigma = 1.0;
    int m_maxIter = 200;
    int m_knn = 10;
    int m_dims = 0;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build RBF similarity matrix */
    QVector<QVector<double>> buildSimilarity(const QVector<QVector<double>>& data) const;

    /** @brief Build normalized Laplacian (symmetric) */
    void buildNormalizedLaplacian(const QVector<QVector<double>>& W,
                                   QVector<QVector<double>>& L) const;

    /** @brief Power iteration to extract top-k eigenvectors */
    QVector<QVector<double>> extractEigenvectors(
        const QVector<QVector<double>>& L, int k) const;

    /** @brief K-means discretization on spectral embeddings */
    QVector<int> kmeansDiscretize(const QVector<QVector<double>>& emb, int k) const;

    /** @brief Squared Euclidean distance */
    double squaredDist(const QVector<double>& a, const QVector<double>& b) const;
};
