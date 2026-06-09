/**
 * @file KMedoids20.h
 * @brief K中心点聚类(BanditPAM估计距离+贪心初始化优化交换) — K-Medoids with BanditPAM Estimated Distances and Optimized Swap with Greedy Initialization for Large Datasets
 *
 * 功能: 实现K-Medoids聚类算法，使用BanditPAM(Bandit-based Partitioning
 *       Around Medoids)多臂老虎机策略估计距离减少计算量，贪心初始化
 *       (greedy initialization)选择初始中心点，优化交换策略(swap
 *       optimization)在大数据集上高效迭代。
 *
 * 协作: GaussianMixture26(高斯混合模型) / KMeans25(K-means) / SpectralCluster13(谱聚类)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief K中心点聚类(BanditPAM估计距离+贪心初始化)
 */
class KMedoids20 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numSamples = 0;
        int numMedoids = 0;
        int numIterations = 0;
        int numSwaps = 0;
        int numDistanceEstimates = 0;
        double totalLoss = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit KMedoids20(QObject *parent = nullptr);
    ~KMedoids20() override;

    /** @brief Set number of medoids and max iterations */
    void setParams(int k, int maxIter);

    /** @brief Set BanditPAM confidence parameter */
    void setBanditConfidence(double delta);

    /** @brief Fit model to data, return cluster assignments */
    QVector<int> fit(const QVector<QVector<double>>& data);

    /** @brief Get medoid indices from last fit */
    QVector<int> medoids() const;

    /** @brief Compute total loss (sum of distances to medoid) */
    double totalLoss() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void fitCompleted(int k, double loss, double timeMs);

private:
    int m_k = 3;
    int m_maxIter = 100;
    double m_delta = 0.05;  // Bandit confidence

    int m_n = 0;            // Number of samples
    int m_d = 0;            // Dimensions
    QVector<int> m_medoids; // Current medoid indices
    QVector<double> m_lossPerMedoid;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Pairwise distance cache (N x N upper triangle) */
    QVector<QVector<double>> m_distCache;

    /** @brief Compute Euclidean distance between two points */
    double distance(const QVector<QVector<double>>& data, int i, int j) const;

    /** @brief Precompute all pairwise distances */
    void buildDistanceCache(const QVector<QVector<double>>& data);

    /** @brief Greedy initialization: select medoids sequentially */
    void greedyInit(const QVector<QVector<double>>& data);

    /** @brief BanditPAM: estimate best swap using confidence bounds */
    int banditSwap(const QVector<QVector<double>>& data, int medoidIdx);

    /** @brief Compute assignment loss for current medoids */
    double computeLoss(const QVector<QVector<double>>& data) const;

    /** @brief Assign each point to nearest medoid */
    QVector<int> assign(const QVector<QVector<double>>& data) const;
};
