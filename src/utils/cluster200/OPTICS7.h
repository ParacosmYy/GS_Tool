/**
 * @file OPTICS7.h
 * @brief OPTICS聚类(梯度法自动簇提取+可达距离熵评分) — OPTICS Clustering with Automated Cluster Extraction via Gradient Method and Reachability Entropy Scoring
 *
 * 功能: 实现OPTICS排序算法，支持梯度法自动簇边界检测、
 *       可达距离熵评分和多层次聚类提取。
 *
 * 协作: DBSCAN11(DBSCAN) / SubspaceCluster6(子空间聚类) / HierarchicalCluster8(层次聚类)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief OPTICS聚类(梯度法自动簇提取+可达距离熵评分)
 */
class OPTICS7 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalFits = 0;
        int numSamples = 0;
        int numClusters = 0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief A cluster extracted from reachability plot */
    struct Cluster {
        int start = 0;
        int end = 0;
        double entropy = 0.0;
        double avgReachability = 0.0;
    };

    explicit OPTICS7(QObject *parent = nullptr);
    ~OPTICS7() override;

    void setEpsilon(double eps);
    void setMinPoints(int minPts);
    void setGradientThreshold(double threshold);

    /** @brief Run OPTICS on n x d data matrix, returns cluster labels */
    QVector<int> fit(const QVector<QVector<double>>& data);

    /** @brief Get reachability distances in processing order */
    QVector<double> reachabilityPlot() const;

    /** @brief Get OPTICS ordering (point indices) */
    QVector<int> ordering() const;

    /** @brief Extract clusters via gradient method on reachability plot */
    QVector<Cluster> extractClusters(const QVector<double>& reachability) const;

    /** @brief Compute reachability entropy for a cluster region */
    double computeEntropy(const QVector<double>& reachability, int start, int end) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void clusteringCompleted(int clusters, double entropy, double timeMs);

private:
    double m_eps = 1.0;
    int m_minPts = 5;
    double m_gradientThreshold = 0.5;

    QVector<double> m_reachability;
    QVector<int> m_ordering;
    QVector<int> m_coreDist;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Euclidean distance between two points */
    static double distance(const QVector<double>& a, const QVector<double>& b);

    /** @brief Compute core distance for a point */
    double coreDistance(const QVector<QVector<double>>& data, int idx,
                        const QVector<int>& neighbors) const;

    /** @brief Find epsilon-neighborhood */
    QVector<int> regionQuery(const QVector<QVector<double>>& data, int idx) const;

    /** @brief Update seeds in priority queue */
    void updateSeeds(const QVector<QVector<double>>& data, int idx,
                      const QVector<int>& neighbors,
                      QVector<double>& reachDist, QVector<bool>& processed,
                      QVector<int>& predecessor) const;
};
