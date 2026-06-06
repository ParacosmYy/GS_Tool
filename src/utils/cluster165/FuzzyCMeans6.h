/**
 * @file FuzzyCMeans6.h
 * @brief 模糊C均值聚类(Gustafson-Kessel扩展+聚类有效性指标) — Fuzzy C-Means with Gustafson-Kessel Extension and Cluster Validity Indices
 *
 * 功能: 实现模糊C均值(FCM)聚类算法，支持Gustafson-Kessel协方差矩阵扩展，
 *       提供Xie-Beni、Partition Coefficient、Partition Entropy等聚类有效性指标。
 *
 * 协作: BirchClustering5(BIRCH聚类) / GaussianMixture11(GMM) / KMedoids13(PAM聚类)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 模糊C均值聚类器(GK扩展)
 */
class FuzzyCMeans6 : public QObject {
    Q_OBJECT

public:
    /** @brief 聚类有效性指标 */
    struct ValidityIndices {
        double xieBeni = 0.0;           ///< Xie-Beni指数(越小越好)
        double partitionCoeff = 0.0;    ///< 划分系数(越大越好)
        double partitionEntropy = 0.0;  ///< 划分熵(越小越好)
    };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalRuns = 0;           ///< 累计运行次数
        int lastClusterCount = 0;        ///< 最近一次簇数
        int lastIterations = 0;          ///< 最近一次迭代数
        double avgProcessingTimeMs = 0.0;///< 平均处理耗时(ms)
    };

    explicit FuzzyCMeans6(QObject* parent = nullptr);
    ~FuzzyCMeans6() override;

    /** @brief 设置模糊指数m(默认2.0) */
    void setFuzziness(double m);

    /** @brief 设置最大迭代次数 */
    void setMaxIterations(int maxIter);

    /** @brief 设置收敛阈值 */
    void setTolerance(double eps);

    /**
     * @brief 执行FCM聚类(标准欧氏距离)
     * @param data 数据集(每行一个样本)
     * @param c 簇数
     * @return 每个样本的簇标签(最大隶属度)
     */
    QVector<int> fit(const QVector<QVector<double>>& data, int c);

    /**
     * @brief 执行Gustafson-Kessel扩展FCM
     * @param data 数据集
     * @param c 簇数
     * @return 簇标签
     */
    QVector<int> fitGustafsonKessel(const QVector<QVector<double>>& data, int c);

    /** @brief 获取隶属度矩阵(U: c x n) */
    QVector<QVector<double>> membershipMatrix() const;

    /** @brief 获取聚类中心 */
    QVector<QVector<double>> centroids() const;

    /** @brief 计算聚类有效性指标 */
    ValidityIndices computeValidity() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 聚类完成 @param c 簇数 @param iters 迭代数 */
    void clusteringCompleted(int c, int iters);

private:
    /** @brief 初始化隶属度矩阵(随机) */
    void initMembership(int n, int c);

    /** @brief 更新聚类中心 */
    void updateCentroids(const QVector<QVector<double>>& data);

    /** @brief 更新隶属度矩阵(欧氏距离) */
    void updateMembership(const QVector<QVector<double>>& data);

    /** @brief 更新隶属度矩阵(GK协方差距离) */
    void updateMembershipGK(const QVector<QVector<double>>& data);

    /** @brief 计算两个向量之间的欧氏距离 */
    static double euclidean(const QVector<double>& a, const QVector<double>& b);

    /** @brief 计算Frobenius范数变化判断收敛 */
    double membershipChange() const;

    double m_fuzziness = 2.0;
    int m_maxIter = 200;
    double m_tolerance = 1e-5;

    QVector<QVector<double>> m_U;        ///< 隶属度矩阵 c x n
    QVector<QVector<double>> m_centers;  ///< 聚类中心 c x dim
    QVector<QVector<double>> m_data;     ///< 数据缓存

    Stats m_stats;
    double m_timeSum = 0.0;
};
