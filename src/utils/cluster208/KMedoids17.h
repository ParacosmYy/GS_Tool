/**
 * @file KMedoids17.h
 * @brief K-中心点聚类(CLARA采样+BanditPAM线性时间中心点搜索) — K-Medoids Clustering with CLARA Sampling and BanditPAM Linear-Time Medoid Search
 *
 * 功能: 实现K-中心点聚类算法，支持CLARA采样策略、
 *       BanditPAM线性时间中心点搜索和轮廓系数评估。
 *
 * 协作: GaussianMixture17(高斯混合) / KMeans19(K均值) / SpectralCluster10(谱聚类)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief K-中心点聚类(CLARA采样+BanditPAM线性时间中心点搜索)
 */
class KMedoids17 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numMedoids = 0;
        int numSamples = 0;
        int numDimensions = 0;
        double silhouetteScore = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit KMedoids17(QObject *parent = nullptr);
    ~KMedoids17() override;

    void setNumMedoids(int k);
    void setMaxIterations(int maxIter);
    void setSampleSize(int sampleSize);
    void setNumSamples(int numSamples);

    /** @brief Compute pairwise distance matrix */
    static QVector<QVector<double>> computeDistanceMatrix(
        const QVector<QVector<double>>& data);

    /** @brief Fit model using full dataset with BanditPAM */
    void fit(const QVector<QVector<double>>& data);

    /** @brief Fit using CLARA sampling strategy */
    void fitCLARA(const QVector<QVector<double>>& data);

    /** @brief Assign each point to nearest medoid */
    QVector<int> predict(const QVector<QVector<double>>& data) const;

    /** @brief Compute silhouette coefficient */
    double silhouetteScore(const QVector<QVector<double>>& data) const;

    /** @brief Get medoid indices */
    QVector<int> medoids() const;

    /** @brief Get cluster labels */
    QVector<int> labels() const;

    /** @brief Compute total cost (sum of distances to medoids) */
    double totalCost() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void fittingCompleted(int medoids, double cost, double timeMs);

private:
    int m_k = 3;
    int m_maxIter = 100;
    int m_sampleSize = 40;       // CLARA sample size per draw
    int m_numSamples = 5;        // Number of CLARA sampling rounds

    QVector<int> m_medoids;      // Indices of current medoids
    QVector<int> m_labels;       // Cluster assignments
    QVector<QVector<double>> m_dist;  // Distance matrix cache

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build step: select initial medoids via BanditPAM */
    QVector<int> buildStep(int n) const;

    /** @brief Swap step: iteratively improve medoid set */
    bool swapStep(int n, QVector<int>& meds) const;

    /** @brief Assign labels from medoids using distance matrix */
    QVector<int> assignLabels(int n, const QVector<int>& meds) const;

    /** @brief Compute cost of a medoid configuration */
    double computeCost(int n, const QVector<int>& meds) const;

    /** @brief Compute target distances for BanditPAM arm-pulling */
    QVector<double> computeTargetDistances(
        int n, const QVector<int>& meds) const;

    /** @brief Euclidean distance between two points */
    static double euclidean(const QVector<double>& a,
                            const QVector<double>& b);
};
