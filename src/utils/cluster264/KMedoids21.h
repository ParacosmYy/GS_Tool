/**
 * @file KMedoids21.h
 * @brief K-中心点聚类(PAM划分式BUILD-SWAP启发式鲁棒基于样本的聚类) — K-Medoids with PAM Partitioning Around Medoids and BUILD-SWAP Heuristic for Robust Exemplar-Based Clustering
 *
 * 功能: 实现K-中心点聚类(K-medoids)，采用PAM划分式算法(Partitioning Around Medoids)
 *       和BUILD-SWAP启发式(BUILD-SWAP heuristic)实现鲁棒基于样本的聚类。
 *
 * 协作: GaussianMixture29(高斯混合) / KMeans27(K均值) / SpectralCluster14(谱聚类)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief K-中心点聚类(PAM划分式BUILD-SWAP启发式鲁棒基于样本的聚类)
 */
class KMedoids21 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numMedoids = 0;
        int numPoints = 0;
        int dimension = 0;
        double totalCost = 0.0;
        int swapCount = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit KMedoids21(QObject *parent = nullptr);
    ~KMedoids21() override;

    /** @brief Set number of medoids and max swap iterations */
    void setParameters(int numMedoids, int maxIterations = 100);

    /** @brief Build phase: greedy select initial medoids */
    void buildPhase(const QVector<QVector<double>>& data);

    /** @brief Swap phase: iteratively improve medoid set */
    void swapPhase(const QVector<QVector<double>>& data);

    /** @brief Full PAM fit: build then swap */
    bool fit(const QVector<QVector<double>>& data);

    /** @brief Assign each point to nearest medoid */
    QVector<int> predict(const QVector<QVector<double>>& data) const;

    /** @brief Get current medoid indices */
    QVector<int> medoidIndices() const;

    /** @brief Compute total assignment cost */
    double totalCost() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void clusteringUpdated(int numMedoids, double cost, int swaps, double timeMs);

private:
    int m_K = 3;
    int m_maxIter = 100;
    int m_n = 0;
    int m_dim = 0;

    QVector<QVector<double>> m_data;
    QVector<int> m_medoidIdx;       // Indices of current medoids
    QVector<int> m_labels;          // Assignment of each point
    QVector<double> m_distances;    // Distance to nearest medoid

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Euclidean distance between two points */
    double euclidean(const QVector<double>& a, const QVector<double>& b) const;

    /** @brief Precompute pairwise distance matrix */
    QVector<QVector<double>> computeDistanceMatrix(
        const QVector<QVector<double>>& data) const;

    /** @brief Assign all points to nearest medoid */
    void assignToMedoids(const QVector<QVector<double>>& distMatrix);

    /** @brief Compute cost of swapping medoid j with non-medoid i */
    double swapCost(const QVector<QVector<double>>& distMatrix, int medIdx, int pointIdx);
};
