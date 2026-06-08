/**
 * @file KMedoids18.h
 * @brief K-中心点聚类(Voronoi交替迭代+Bandit臂抽取评估) — K-Medoids Clustering with Alternating Voronoi Iteration and Bandit-based Arm Pulling for Medoid Evaluation
 *
 * 功能: 实现K-Medoids聚类算法，采用交替Voronoi划分与中心点更新，
 *       集成Bandit策略加速中心点候选评估，降低距离计算开销。
 *
 * 协作: GaussianMixture20(高斯混合) / KMeans21(K均值) / FuzzyCMeans10(模糊聚类)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief K-中心点聚类(Voronoi+Bandit评估)
 */
class KMedoids18 : public QObject {
    Q_OBJECT

public:
    /** @brief Clustering result */
    struct MedoidResult {
        QVector<int> assignments;
        QVector<int> medoidIndices;
        QVector<double> medoidCosts;
        double totalCost = 0.0;
        int iterations = 0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numPoints = 0;
        int numMedoids = 0;
        int dim = 0;
        int totalIterations = 0;
        quint64 distanceComputations = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit KMedoids18(QObject *parent = nullptr);
    ~KMedoids18() override;

    /** @brief Set parameters: number of medoids, max iterations, bandit samples */
    void setParameters(int numMedoids = 5, int maxIterations = 100,
                       int banditSamples = 10);

    /** @brief Fit medoids to data */
    MedoidResult fit(const QVector<QVector<double>>& data);

    /** @brief Assign new points to nearest medoid */
    QVector<int> predict(const QVector<QVector<double>>& data) const;

    /** @brief Get current medoid indices */
    QVector<int> medoidIndices() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void clusteringCompleted(int medoids, int iterations, double cost, double timeMs);

private:
    int m_numMedoids = 5;
    int m_maxIterations = 100;
    int m_banditSamples = 10;

    QVector<int> m_medoidIndices;
    QVector<QVector<double>> m_data;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute pairwise distance between two points */
    double distance(const QVector<double>& a, const QVector<double>& b) const;

    /** @brief Compute total medoid cost (sum of distances to nearest medoid) */
    double computeTotalCost(const QVector<int>& medoids,
                             const QVector<QVector<double>>& data) const;

    /** @brief Build Voronoi assignments for given medoids */
    QVector<int> buildVoronoi(const QVector<int>& medoids,
                                const QVector<QVector<double>>& data) const;

    /** @brief Bandit-based medoid swap evaluation */
    int banditSelectCandidate(int medoidIdx, const QVector<int>& medoids,
                                const QVector<QVector<double>>& data) const;

    /** @brief Initialize medoids via BUILD step (greedy) */
    QVector<int> buildInitialMedoids(const QVector<QVector<double>>& data) const;
};
