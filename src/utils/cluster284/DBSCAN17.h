/**
 * @file DBSCAN17.h
 * @brief DBSCAN密度聚类(边界点重分配与变epsilon密度自适应边界精化) — DBSCAN with Border Point Reassignment and Variable Epsilon Radius for Density-adaptive Cluster Boundary Refinement
 *
 * 功能: 实现DBSCAN密度聚类算法，采用边界点重分配(border point reassignment)
 *       与变epsilon半径(variable epsilon radius)实现密度自适应簇边界精化(density-adaptive cluster boundary refinement)。
 *
 * 协作: KMeans30(K-means聚类) / OPTICS13(光学聚类) / GaussianMixture33(高斯混合)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief DBSCAN密度聚类(边界点重分配与变epsilon密度自适应边界精化)
 */
class DBSCAN17 : public QObject {
    Q_OBJECT

public:
    /** @brief Clustering result */
    struct ClusterResult {
        QVector<int> labels;           // -1 = noise
        QVector<QVector<int>> clusters;
        int numClusters = 0;
        int numNoise = 0;
        int iterations = 0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numPoints = 0;
        int numClusters = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit DBSCAN17(QObject *parent = nullptr);
    ~DBSCAN17() override;

    void setEpsilon(double eps);
    void setMinPoints(int minPts);
    void setEpsilonRatio(double ratio);
    void setMaxBorderIter(int iters);

    /** @brief Run DBSCAN with variable epsilon and border reassignment */
    ClusterResult fit(const QVector<QVector<double>>& data);

    /** @brief Predict cluster for new samples using nearest core point */
    QVector<int> predict(const QVector<QVector<double>>& samples,
                         const QVector<QVector<double>>& trainData) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void fitDone(int n, int clusters, int noise, double timeMs);

private:
    double m_eps = 0.5;
    int m_minPts = 5;
    double m_epsRatio = 1.5;     // Border reassignment epsilon multiplier
    int m_maxBorderIter = 3;
    Stats m_stats;
    double m_timeSum = 0.0;

    QVector<int> m_labels;
    QVector<bool> m_visited;
    QVector<bool> m_isCore;
    QVector<QVector<double>> m_data;

    /** @brief Euclidean distance between two points */
    double distance(const QVector<double>& a, const QVector<double>& b) const;

    /** @brief Range query: find neighbors within eps */
    QVector<int> regionQuery(int idx, double eps) const;

    /** @brief Expand cluster from core point */
    void expandCluster(int idx, int clusterId, double eps);

    /** @brief Compute local density for variable epsilon */
    double localDensity(int idx) const;

    /** @brief Border point reassignment pass */
    void reassignBorders();

    /** @brief Get variable epsilon for a point */
    double variableEpsilon(int idx) const;
};
