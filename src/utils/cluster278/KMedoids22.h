/**
 * @file KMedoids22.h
 * @brief K-中心点聚类(CLARA采样与多起点PAM的大规模可扩展中心点聚类) — K-medoids with CLARA Sampling and Multi-start PAM for Scalable Large-dataset Medoid-based Clustering
 *
 * 功能: 实现K-中心点聚类(K-medoids)，采用CLARA采样(CLARA sampling)
 *       与多起点PAM(multi-start PAM)实现大规模可扩展中心点聚类(scalable large-dataset medoid-based clustering)。
 *
 * 协作: GaussianMixture32(高斯混合) / KMeans29(K均值) / DBSCAN16(密度聚类)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief K-中心点聚类(CLARA采样与多起点PAM)
 */
class KMedoids22 : public QObject {
    Q_OBJECT

public:
    /** @brief Clustering result */
    struct MedoidResult {
        QVector<int> medoidIndices;
        QVector<int> labels;
        double totalCost = 0.0;
        int iterations = 0;
        bool converged = false;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numPoints = 0;
        int numMedoids = 0;
        double avgProcessingTimeMs = 0.0;
        int claraSamples = 0;
    };

    explicit KMedoids22(QObject *parent = nullptr);
    ~KMedoids22() override;

    /** @brief Set number of medoids k */
    void setK(int k);

    /** @brief Set max PAM iterations per start */
    void setMaxIterations(int iter);

    /** @brief Set number of CLARA sampling rounds */
    void setClaraSamples(int samples);

    /** @brief Set sample size fraction for CLARA (0..1) */
    void setSampleFraction(double frac);

    /** @brief Set number of multi-start PAM runs */
    void setMultiStarts(int starts);

    /** @brief Fit using full PAM (small datasets) */
    MedoidResult fitPAM(const QVector<QVector<double>>& data);

    /** @brief Fit using CLARA sampling (large datasets) */
    MedoidResult fitCLARA(const QVector<QVector<double>>& data);

    /** @brief Assign new points to nearest medoid */
    QVector<int> assign(const QVector<QVector<double>>& data,
                        const QVector<int>& medoidIdx) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void fittingDone(int k, double cost, double timeMs);
    void pamStartDone(int startIdx, double cost);
    void claraRoundDone(int round, double bestCost);

private:
    int m_k = 3;
    int m_maxIter = 100;
    int m_claraSamples = 5;
    double m_sampleFrac = 0.3;
    int m_multiStarts = 3;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Precomputed distance matrix */
    QVector<QVector<double>> m_dist;

    /** @brief Compute pairwise distance matrix */
    void buildDistMatrix(const QVector<QVector<double>>& data);

    /** @brief Euclidean distance between two points */
    double euclidean(const QVector<double>& a, const QVector<double>& b) const;

    /** @brief Run single PAM on precomputed distances (indices into data) */
    MedoidResult runPAM(const QVector<int>& pointIdx, int k) const;

    /** @brief Compute total cost for given medoid set */
    double totalCost(const QVector<int>& medoids, const QVector<int>& pointIdx) const;

    /** @brief Assign points to nearest medoid, return labels */
    QVector<int> assignLabels(const QVector<int>& medoids,
                               const QVector<int>& pointIdx) const;
};
