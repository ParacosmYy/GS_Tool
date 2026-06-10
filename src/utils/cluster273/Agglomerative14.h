/**
 * @file Agglomerative14.h
 * @brief 层次凝聚聚类(灵活Lance-Williams更新公式与共表相关性树状图质量评估) — Agglomerative Clustering with Flexible Lance-Williams Update Formula and Cophenetic Correlation for Dendrogram Quality Assessment
 *
 * 功能: 实现层次凝聚聚类(Agglomerative clustering)，采用灵活Lance-Williams更新公式
 *       (flexible Lance-Williams update formula)与共表相关性(cophenetic correlation)
 *       实现树状图质量评估(dendrogram quality assessment)。
 *
 * 协作: KMeans29(K均值) / DBSCAN16(密度聚类) / GaussianMixture31(高斯混合)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 层次凝聚聚类(灵活Lance-Williams更新公式与共表相关性)
 */
class Agglomerative14 : public QObject {
    Q_OBJECT

public:
    /** @brief Linkage method for Lance-Williams formula */
    enum Linkage {
        SingleLinkage = 0,   // α_i=0.5, α_j=0.5, β=0, γ=-0.5
        CompleteLinkage,     // α_i=0.5, α_j=0.5, β=0, γ=0.5
        AverageLinkage,      // UPGMA
        WardLinkage,         // Ward's minimum variance
        FlexibleLinkage      // User-tunable β, γ parameters
    };

    /** @brief A merge step in the dendrogram */
    struct MergeStep {
        int clusterI = -1;     // First merged cluster index
        int clusterJ = -1;     // Second merged cluster index
        double distance = 0.0; // Distance at merge
        int newSize = 0;       // Resulting cluster size
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numPoints = 0;
        int numMerges = 0;
        double copheneticCorrelation = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Agglomerative14(QObject *parent = nullptr);
    ~Agglomerative14() override;

    /** @brief Set linkage method */
    void setLinkage(Linkage method);

    /** @brief Set flexible Lance-Williams parameters (β, γ) */
    void setFlexibleParams(double beta, double gamma);

    /** @brief Set convergence: stop when nClusters reached */
    void setTargetClusters(int n);

    /** @brief Run clustering, returns cluster labels per point */
    QVector<int> fit(const QVector<QVector<double>>& data);

    /** @brief Get full dendrogram (merge steps) */
    QVector<MergeStep> dendrogram() const;

    /** @brief Compute cophenetic correlation coefficient */
    double copheneticCorrelation(const QVector<QVector<double>>& data);

    /** @brief Cut dendrogram at given distance threshold */
    QVector<int> cutAtDistance(double threshold) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void clusteringDone(int numClusters, double cophCorr, double timeMs);

private:
    Linkage m_linkage = WardLinkage;
    double m_beta = 0.0;       // Flexible β parameter
    double m_gamma = 0.0;      // Flexible γ parameter
    int m_targetClusters = 2;

    QVector<MergeStep> m_merges;  // Dendrogram merge history
    QVector<double> m_originalDist; // Original pairwise distances

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute condensed pairwise Euclidean distance matrix */
    QVector<double> pairwiseDistances(const QVector<QVector<double>>& data) const;

    /** @brief Compute Lance-Williams coefficients for current linkage */
    void lanceWilliamsCoeffs(int ni, int nj, int nk,
                             double& ai, double& aj, double& b, double& g) const;

    /** @brief Find minimum distance index in condensed matrix */
    int findMinDist(const QVector<double>& dist, const QVector<bool>& active, int n) const;

    /** @brief Build original distance vector for cophenetic computation */
    void buildOriginalDistances(const QVector<QVector<double>>& data);
};
