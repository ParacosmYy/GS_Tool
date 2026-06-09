/**
 * @file KMedoids19.h
 * @brief K-中心点聚类(CLARA采样+PAM构建交换启发式) — K-Medoids with CLARA Sampling and PAM Build-Swap Heuristic for Scalable Medoid Selection
 *
 * 功能: 实现K-中心点聚类算法(K-medoids)，采用CLARA采样(CLARA sampling)从大数据集中
 *       抽取多个子集进行PAM分析，结合PAM构建交换启发式(PAM build-swap heuristic)在
 *       每个子集上快速选择最优中心点(medoid selection)，实现可扩展聚类。
 *
 * 协作: GaussianMixture23(高斯混合) / KMeans23(K均值) / BirchClustering11(BIRCH聚类)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief K-中心点聚类(CLARA采样+PAM构建交换启发式)
 */
class KMedoids19 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numPoints = 0;
        int numDimensions = 0;
        int numMedoids = 0;
        int claraSamples = 0;
        int totalSwapEvals = 0;
        double bestCost = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit KMedoids19(QObject *parent = nullptr);
    ~KMedoids19() override;

    /** @brief Set number of medoids k */
    void setNumMedoids(int k);

    /** @brief Set CLARA sample size per iteration */
    void setSampleSize(int size);

    /** @brief Set number of CLARA sampling iterations */
    void setNumSamples(int n);

    /** @brief Set max PAM swap iterations per sample */
    void setMaxSwapIter(int iter);

    /** @brief Fit model to [n x d] data, returns medoid indices */
    QVector<int> fit(const QVector<QVector<double>>& data);

    /** @brief Predict nearest medoid for a new point */
    int predict(const QVector<double>& point) const;

    /** @brief Get current medoid indices */
    QVector<int> medoids() const;

    /** @brief Get cluster assignments for all data */
    QVector<int> labels() const;

    /** @brief Compute total dissimilarity cost */
    double totalCost() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void claraSampleCompleted(int sampleIdx, double cost, double timeMs);
    void fitCompleted(int k, double bestCost, double totalTimeMs);

private:
    int m_k = 5;
    int m_sampleSize = 40;
    int m_numSamples = 5;
    int m_maxSwapIter = 100;

    QVector<QVector<double>> m_data;
    QVector<int> m_medoids;
    QVector<int> m_labels;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Pairwise distance between two points */
    double distance(const QVector<double>& a, const QVector<double>& b) const;

    /** @brief Compute pairwise distance matrix for subset */
    QVector<QVector<double>> distanceMatrix(const QVector<QVector<double>>& pts) const;

    /** @brief PAM BUILD phase: greedy select initial medoids */
    QVector<int> pamBuild(const QVector<QVector<double>>& dist, int k) const;

    /** @brief PAM SWAP phase: iteratively improve medoid set */
    double pamSwap(const QVector<QVector<double>>& dist, QVector<int>& meds) const;

    /** @brief Assign each point to nearest medoid */
    void assignLabels();

    /** @brief Draw random sample of given size */
    QVector<int> drawSample(int n, int sampleSize) const;
};
