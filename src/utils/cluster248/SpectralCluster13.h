/**
 * @file SpectralCluster13.h
 * @brief 谱聚类(Shi-Malik归一化割+旋转对齐特征向量离散化) — Spectral Clustering with Shi-Malik Normalized Cut and Eigenvector Discretization via Rotation Alignment
 *
 * 功能: 实现谱聚类算法(Spectral Clustering)，使用Shi-Malik归一化割准则
 *       (normalized cut criterion)构造图拉普拉斯矩阵，通过特征向量分解
 *       (eigendecomposition)获得嵌入空间，旋转对齐(rotation alignment)
 *       实现特征向量离散化得到最终聚类标签。
 *
 * 协作: FuzzyCMeans12(模糊C均值) / KMeans25(K-means) / GaussianMixture25(高斯混合)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 谱聚类(Shi-Malik归一化割+旋转对齐离散化)
 */
class SpectralCluster13 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numSamples = 0;
        int numDimensions = 0;
        int numClusters = 0;
        int numIterations = 0;
        double ncutValue = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SpectralCluster13(QObject *parent = nullptr);
    ~SpectralCluster13() override;

    /** @brief Set number of clusters K */
    void setNumClusters(int k);

    /** @brief Set RBF kernel sigma and k-nearest-neighbor count */
    void setKernelParams(double sigma, int knn);

    /** @brief Set max iterations for discretization */
    void setMaxIterations(int maxIter);

    /** @brief Run spectral clustering, return labels */
    QVector<int> fit(const QVector<QVector<double>>& data);

    /** @brief Get eigenvectors from last run (N x K) */
    QVector<QVector<double>> eigenvectors() const;

    /** @brief Compute normalized cut value for current partition */
    double ncutValue() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void clusteringCompleted(int numClusters, double ncut, double timeMs);

private:
    int m_numClusters = 3;
    double m_sigma = 1.0;
    int m_knn = 10;
    int m_maxIter = 100;

    QVector<QVector<double>> m_data;       // N x D
    QVector<QVector<double>> m_similarity; // N x N
    QVector<QVector<double>> m_eigvecs;    // N x K
    QVector<int> m_labels;
    double m_ncutValue = 0.0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build similarity matrix with RBF kernel + kNN sparsification */
    void buildSimilarityMatrix();

    /** @brief Solve generalized eigenproblem: Ly = λDy */
    void solveEigensystem();

    /** @brief Normalize eigenvectors row-wise */
    void normalizeEigenvectors();

    /** @brief Discretize via rotation alignment */
    void discretizeRotationAlignment();

    /** @brief Compute Euclidean distance */
    double distance(const QVector<double>& a,
                    const QVector<double>& b) const;

    /** @brief Compute rotation objective Z*Q and align */
    void alignRotation(QVector<QVector<double>>& rotation);
};
