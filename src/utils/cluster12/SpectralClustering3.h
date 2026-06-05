/**
 * @file SpectralClustering3.h
 * @brief 谱聚类算法 — 归一化切割 + Laplacian特征分解 + K-Means
 *
 * 功能: 构建相似度图 → 归一化Laplacian → 取前k个特征向量 → K-Means聚类。
 *       支持RBF/KNN相似度矩阵、归一化切割、多尺度参数，统计聚类次数/特征分解耗时。
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @class SpectralClustering3
 * @brief 基于谱图理论的聚类器，支持归一化切割
 */
class SpectralClustering3 : public QObject {
    Q_OBJECT
public:
    /** 聚类统计 */
    struct Stats {
        quint64 totalClusterings = 0;       ///< 总聚类次数
        quint64 totalEigenComputations = 0; ///< 累计特征分解次数
        quint64 totalSamplesProcessed = 0;  ///< 累计处理样本数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理耗时(ms)
    };

    /** 单次聚类结果 */
    struct ClusterResult {
        QVector<int> labels;                ///< 每个样本的簇标签(0~k-1)
        double normalizedCut = 0.0;         ///< 归一化切割值
        double eigenGap = 0.0;              ///< 特征间隙(用于自动选k)
        int iterations = 0;                 ///< K-Means迭代次数
    };

    /** 构造函数 */
    explicit SpectralClustering3(QObject* parent = nullptr);

    /** @brief 设置簇数量 @param k 目标簇数 */
    void setK(int k);

    /** @brief 设置RBF核宽度(sigma) @param sigma 高斯核参数 */
    void setSigma(double sigma);

    /** @brief 设置KNN近邻数 @param knn K近邻数 */
    void setKNN(int knn);

    /** @brief 设置最大K-Means迭代 @param maxIter 最大迭代 */
    void setMaxIterations(int maxIter);

    /**
     * @brief 执行谱聚类
     * @param data 二维数据[n_samples][n_features]
     * @return 聚类结果
     */
    ClusterResult cluster(const QVector<QVector<double>>& data);

    /**
     * @brief 自动选择最优k(基于特征间隙)
     * @param data 输入数据
     * @param maxK 最大候选k
     * @return 推荐簇数
     */
    int autoSelectK(const QVector<QVector<double>>& data, int maxK = 10) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 聚类完成 @param k 簇数 @param ncut 归一化切割值 */
    void clusteringComplete(int k, double ncut);
    /** @brief 特征分解完成 @param n 矩阵阶数 @param nEigen 提取特征数 */
    void eigenDecomposed(int n, int nEigen);

private:
    /** 构建RBF相似度矩阵 */
    QVector<QVector<double>> buildRBFAffinity(const QVector<QVector<double>>& data) const;
    /** 构建KNN相似度矩阵 */
    QVector<QVector<double>> buildKNNAffinity(const QVector<QVector<double>>& data) const;
    /** 计算归一化Laplacian特征向量(幂迭代) */
    QVector<QVector<double>> computeEigenvectors(
        const QVector<QVector<double>>& laplacian, int k) const;
    /** K-Means子程序 */
    QPair<QVector<int>, int> runKMeans(const QVector<QVector<double>>& points, int k) const;
    /** 计算归一化切割 */
    double computeNCut(const QVector<QVector<double>>& affinity,
                       const QVector<int>& labels, int k) const;
    /** 向量点积 */
    static double dotProduct(const QVector<double>& a, const QVector<double>& b);
    /** 向量归一化 */
    static void normalizeVec(QVector<double>& v);

    int    m_k;             ///< 目标簇数
    double m_sigma;         ///< RBF核宽度
    int    m_knn;           ///< KNN近邻数
    int    m_maxIterations; ///< 最大迭代
    Stats  m_stats;         ///< 统计信息
    double m_timeSum = 0.0; ///< 累计耗时
};
