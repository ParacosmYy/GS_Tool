/**
 * @file SpectralCluster7.h
 * @brief 谱聚类(归一化割+特征间隙自动选K+Nystrom近似) — Spectral Clustering with Normalized Cut, Eigengap Heuristic for Auto-K and Nystrom Approximation
 *
 * 功能: 实现谱聚类算法，支持归一化拉普拉斯矩阵、特征间隙启发式自动选择K值、
 *       Nystrom近似加速大规模数据聚类和K-means嵌入空间划分。
 *
 * 协作: FuzzyCMeans7(模糊C均值) / KMedoids14(K-Medoids) / BirchClustering6(BIRCH)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 谱聚类器(归一化割+特征间隙+Nystrom近似)
 */
class SpectralCluster7 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalRuns = 0;
        int numClusters = 0;
        int numSamples = 0;
        int eigenDims = 0;
        double eigengap = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SpectralCluster7(QObject *parent = nullptr);
    ~SpectralCluster7() override;

    void setNumClusters(int k);
    void setAutoK(bool enabled);
    void setMaxK(int maxK);
    void setSigma(double sigma);
    void setNystromRatio(double ratio);
    void setMaxIterations(int iter);

    /** @brief 执行谱聚类，返回样本标签 */
    QVector<int> fit(const QVector<QVector<double>>& data);

    /** @brief 自动选K: 基于特征间隙启发式 */
    int autoSelectK(const QVector<double>& eigenvalues) const;

    /** @brief 获取聚类中心(嵌入空间) */
    QVector<QVector<double>> centers() const;

    /** @brief 获取拉普拉斯特征向量 */
    QVector<QVector<double>> eigenvectors() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void clusteringCompleted(int numClusters, double eigengap);

private:
    int m_numClusters = 3;
    bool m_autoK = false;
    int m_maxK = 10;
    double m_sigma = 1.0;
    double m_nystromRatio = 0.1;
    int m_maxIter = 100;

    int m_n = 0;
    int m_dims = 0;
    QVector<int> m_labels;
    QVector<QVector<double>> m_centers;
    QVector<QVector<double>> m_eigvecs;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief 高斯相似度矩阵 */
    QVector<QVector<double>> buildAffinity(const QVector<QVector<double>>& data) const;

    /** @brief Nystrom近似: 用采样点近似完整特征分解 */
    void nystromApproximation(const QVector<QVector<double>>& data,
                               QVector<QVector<double>>& eigvecs,
                               QVector<double>& eigvals);

    /** @brief 归一化拉普拉斯: L_norm = D^{-1/2} W D^{-1/2} */
    void normalizedLaplacian(QVector<QVector<double>>& W) const;

    /** @brief 幂迭代求前K个特征向量 */
    void powerIteration(const QVector<QVector<double>>& mat,
                         int k, QVector<QVector<double>>& eigvecs,
                         QVector<double>& eigvals);

    /** @brief 行归一化特征向量矩阵 */
    void normalizeRows(QVector<QVector<double>>& mat);

    /** @brief K-means在嵌入空间 */
    QVector<int> kmeansEmbed(const QVector<QVector<double>>& embedded, int k);
};
