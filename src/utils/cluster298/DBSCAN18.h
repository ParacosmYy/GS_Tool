/**
 * @file DBSCAN18.h
 * @brief DBSCAN密度聚类(局部离群因子集成与自适应minPts实现密度比噪声校准聚类) — DBSCAN with Local Outlier Factor Integration and Adaptive MinPts for Density-Ratio Based Clustering with Noise Calibration
 *
 * 功能: 实现DBSCAN密度聚类(DBSCAN density-based clustering)，采用局部离群因子集成(LOF integration)
 *       与自适应minPts(adaptive minPts)实现密度比噪声校准聚类(density-ratio based clustering with noise calibration)。
 *
 * 协作: KMeans32(K均值聚类) / OPTICS14(有序聚类) / GaussianMixture36(高斯混合模型)
 */
#pragma once

#include <QObject>
#include <QVector>

class DBSCAN18 : public QObject {
    Q_OBJECT

public:
    /** @brief Point with LOF score */
    struct PointInfo {
        QVector<double> coordinates;
        int clusterId = -1;       // -1 = unvisited, 0 = noise
        double localDensity = 0.0;
        double lofScore = 1.0;    // LOF: ~1.0 = inlier, >>1 = outlier
        int neighborCount = 0;
    };

    /** @brief Clustering result with noise calibration */
    struct ClusterResult {
        QVector<int> labels;          // cluster assignment per point
        QVector<double> lofScores;    // LOF score per point
        int numClusters = 0;
        int numNoise = 0;
        double avgSilhouette = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalFits = 0;
        int numPoints = 0;
        int dimensions = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit DBSCAN18(QObject *parent = nullptr);
    ~DBSCAN18() override;

    void setEpsilon(double eps);
    void setBaseMinPts(int minPts);
    void setLofThreshold(double threshold);

    /** @brief Fit DBSCAN with LOF-based noise calibration and adaptive minPts */
    ClusterResult fit(const QVector<QVector<double>>& data);

    /** @brief Compute LOF scores for all points */
    QVector<double> computeLOF(const QVector<QVector<double>>& data) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void fitDone(int clusters, int noise, double avgLof, double timeMs);

private:
    double m_eps = 0.5;
    int m_baseMinPts = 5;
    double m_lofThreshold = 2.0;
    int m_dims = 0;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Euclidean distance between two points */
    double distance(const QVector<double>& a, const QVector<double>& b) const;

    /** @brief Find epsilon-neighborhood of point at index */
    QVector<int> regionQuery(const QVector<QVector<double>>& data, int idx) const;

    /** @brief Compute local reachability density for a point */
    double localReachDensity(const QVector<QVector<double>>& data,
                             int idx, const QVector<QVector<int>>& neighborhoods) const;

    /** @brief Compute adaptive minPts for a point based on local density */
    int adaptiveMinPts(int idx, const QVector<double>& densities) const;

    /** @brief Expand cluster from seed point */
    void expandCluster(const QVector<QVector<double>>& data,
                       int pointIdx, int clusterId,
                       QVector<int>& labels, const QVector<QVector<int>>& neighborhoods,
                       const QVector<int>& adaptedMinPts);
};
