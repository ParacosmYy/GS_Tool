/**
 * @file SpectralCluster6.h
 * @brief 谱聚类(归一化拉普拉斯+特征向量k-means) — Spectral Clustering with Normalized Laplacian and K-Means on Eigenvectors
 *
 * 功能: 实现谱聚类算法，构建归一化拉普拉斯矩阵，提取前k个特征向量，
 *       在特征向量空间上执行k-means，支持不同核函数构建相似度图。
 *
 * 协作: FuzzyCMeans6(模糊C均值) / KMedoids13(PAM聚类) / GaussianMixture11(GMM)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 谱聚类器
 */
class SpectralCluster6 : public QObject {
    Q_OBJECT

public:
    /** @brief 核函数类型 */
    enum KernelType { RBF, KNN, EpsilonNeighborhood };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalRuns = 0;           ///< 累计运行次数
        int lastClusterCount = 0;        ///< 最近簇数
        int lastIterations = 0;          ///< 最近k-means迭代数
        double avgProcessingTimeMs = 0.0;///< 平均耗时(ms)
    };

    explicit SpectralCluster6(QObject *parent = nullptr);
    ~SpectralCluster6() override;

    /** @brief 设置核函数类型 */
    void setKernelType(KernelType type);

    /** @brief 设置RBF核带宽sigma */
    void setSigma(double sigma);

    /** @brief 设置KNN近邻数 */
    void setKNN(int k);

    /** @brief 设置k-means最大迭代 */
    void setMaxIterations(int maxIter);

    /**
     * @brief 执行谱聚类
     * @param data 数据集(每行一个样本)
     * @param k 簇数
     * @return 每个样本的簇标签
     */
    QVector<int> fit(const QVector<QVector<double>>& data, int k);

    /** @brief 获取嵌入向量(特征向量) */
    QVector<QVector<double>> embeddings() const;

    /** @brief 获取聚类中心(嵌入空间) */
    QVector<QVector<double>> centroids() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 聚类完成 @param k 簇数 @param iters 迭代数 */
    void clusteringCompleted(int k, int iters);

private:
    /** @brief 构建相似度矩阵W */
    void buildAffinityMatrix(const QVector<QVector<double>>& data);

    /** @brief 构建归一化拉普拉斯 L_sym = I - D^{-1/2} W D^{-1/2} */
    void buildNormalizedLaplacian(int n);

    /** @brief 幂迭代求前k个特征向量 */
    void computeEigenvectors(int n, int k);

    /** @brief k-means在特征向量空间 */
    QVector<int> kmeansOnEmbeddings(int k);

    /** @brief 欧氏距离 */
    static double euclidean(const QVector<double>& a, const QVector<double>& b);

    KernelType m_kernel = RBF;
    double m_sigma = 1.0;
    int m_knn = 5;
    int m_maxIter = 200;

    QVector<QVector<double>> m_W;         ///< 相似度矩阵
    QVector<QVector<double>> m_L;         ///< 归一化拉普拉斯
    QVector<QVector<double>> m_embeddings;///< 特征向量嵌入
    QVector<QVector<double>> m_centroids; ///< k-means中心

    Stats m_stats;
    double m_timeSum = 0.0;
};
