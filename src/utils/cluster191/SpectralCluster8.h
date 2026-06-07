/**
 * @file SpectralCluster8.h
 * @brief 谱聚类(自适应kNN+epsilon相似图+比率割) — Spectral Clustering with Adaptive Similarity Graph (kNN+epsilon) and Ratio-Cut Objective
 *
 * 功能: 实现谱聚类算法，支持自适应kNN+epsilon混合相似图构建、
 *       比率割(Ratio-Cut)目标优化和Laplacian特征向量嵌入。
 *
 * 协作: FuzzyCMeans8(模糊C均值) / KMeans17(K均值) / DBSCAN10(密度聚类)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 谱聚类器(自适应相似图+比率割)
 */
class SpectralCluster8 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalRuns = 0;
        int numPoints = 0;
        int numClusters = 0;
        int graphEdges = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SpectralCluster8(QObject *parent = nullptr);
    ~SpectralCluster8() override;

    void setKnnK(int k);
    void setEpsilon(double eps);
    void setMaxIterations(int iter);
    void setSigma(double sigma);

    /** @brief Fit model to data, return cluster labels */
    QVector<int> fit(const QVector<QVector<double>>& data, int k);

    /** @brief Build adaptive similarity graph (kNN+epsilon hybrid) */
    QVector<QVector<double>> buildGraph(const QVector<QVector<double>>& data) const;

    /** @brief Compute normalized Laplacian */
    QVector<QVector<double>> laplacian(const QVector<QVector<double>>& graph) const;

    /** @brief Get eigenvectors used for embedding */
    QVector<QVector<double>> eigenvectors() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void clusteringCompleted(int clusters, int edges, double timeMs);

private:
    int m_knnK = 7;
    double m_epsilon = 0.5;
    int m_maxIter = 200;
    double m_sigma = 1.0;

    QVector<QVector<double>> m_eigvecs;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Gaussian kernel similarity */
    double gaussianSim(const QVector<double>& a,
                       const QVector<double>& b) const;

    /** @brief Power iteration for top eigenvectors */
    QVector<QVector<double>> powerEigenvectors(
        const QVector<QVector<double>>& mat, int numVecs, int maxIter) const;

    /** @brief Normalize rows of a matrix */
    void normalizeRows(QVector<QVector<double>>& mat) const;

    /** @brief Simple k-means on embedded vectors */
    QVector<int> embeddedKMeans(const QVector<QVector<double>>& embedded,
                                 int k, int maxIter);
};
