/**
 * @file FuzzyCMeans7.h
 * @brief 模糊C均值聚类(Gustafson-Kessel距离+划分熵有效性) — Fuzzy C-means with Gustafson-Kessel Distance and Partition Entropy Validity
 *
 * 功能: 实现模糊C均值(FCM)聚类算法，支持Gustafson-Kessel自适应距离度量、
 *       协方差矩阵估计、模糊隶属度划分和划分熵聚类有效性评估。
 *
 * 协作: BirchClustering6(BIRCH) / KMedoids14(K-Medoids) / DBSCAN9(DBSCAN)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 模糊C均值聚类器(Gustafson-Kessel距离)
 */
class FuzzyCMeans7 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalRuns = 0;
        int numClusters = 0;
        int numSamples = 0;
        int iterations = 0;
        double partitionEntropy = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit FuzzyCMeans7(QObject *parent = nullptr);
    ~FuzzyCMeans7() override;

    void setNumClusters(int c);
    void setFuzziness(double m);
    void setMaxIterations(int iter);
    void setTolerance(double eps);

    /** @brief 执行模糊C均值聚类，返回每个样本的隶属度矩阵 */
    QVector<QVector<double>> fit(const QVector<QVector<double>>& data);

    /** @brief 获取硬聚类标签(最大隶属度) */
    QVector<int> labels() const;

    /** @brief 获取聚类中心 */
    QVector<QVector<double>> centers() const;

    /** @brief 计算划分熵有效性指标 */
    double partitionEntropy() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void clusteringCompleted(int numClusters, double entropy);

private:
    int m_numClusters = 3;
    double m_fuzziness = 2.0;
    int m_maxIter = 100;
    double m_tolerance = 1e-4;

    int m_dims = 0;
    int m_n = 0;
    QVector<QVector<double>> m_U;       ///< Membership matrix [n x c]
    QVector<QVector<double>> m_centers; ///< Cluster centers [c x dims]
    QVector<int> m_labels;

    Stats m_stats;
    double m_timeSum = 0.0;

    void initMembership();
    void updateCenters(const QVector<QVector<double>>& data);
    void updateCovariances(const QVector<QVector<double>>& data,
                           QVector<QVector<QVector<double>>>& cov);
    double gkDistance(const QVector<double>& x, int cluster,
                      const QVector<QVector<QVector<double>>>& covInv);
    void updateMembership(const QVector<QVector<double>>& data,
                          const QVector<QVector<QVector<double>>>& covInv);
    double computeEntropy() const;
};
