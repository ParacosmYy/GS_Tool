/**
 * @file OPTICS13.h
 * @brief OPTICS聚类(增强可达度聚类提取与陡度评估的任意形状聚类边界) — OPTICS with Enhanced Reachability-based Cluster Extraction and Steepness Evaluation for Arbitrary-shape Cluster Boundaries
 *
 * 功能: 实现OPTICS聚类(OPTICS clustering)，采用增强可达度聚类提取(enhanced reachability-based cluster extraction)
 *       与陡度评估(steepness evaluation)实现任意形状聚类边界(arbitrary-shape cluster boundaries)。
 *
 * 协作: DBSCAN16(密度聚类) / SubspaceCluster12(子空间聚类) / HierarchicalCluster14(层次聚类)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief OPTICS聚类(增强可达度聚类提取与陡度评估)
 */
class OPTICS13 : public QObject {
    Q_OBJECT

public:
    /** @brief Ordered point info in the reachability plot */
    struct OpticsPoint {
        int index = -1;
        double reachability = 1e30;
        double coreDistance = 1e30;
    };

    /** @brief Extracted cluster with boundary info */
    struct Cluster {
        QVector<int> pointIndices;
        double startReach = 0.0;
        double endReach = 0.0;
        bool isSteepUp = false;
        bool isSteepDown = false;
    };

    /** @brief Full OPTICS result */
    struct OpticsResult {
        QVector<OpticsPoint> ordering;
        QVector<Cluster> clusters;
        int numProcessed = 0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numPoints = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit OPTICS13(QObject *parent = nullptr);
    ~OPTICS13() override;

    void setEpsilon(double eps);
    void setMinPoints(int minPts);
    void setSteepnessThreshold(double xi);

    OpticsResult fit(const QVector<QVector<double>>& data);

    double euclideanDist(const QVector<double>& a,
                         const QVector<double>& b) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void orderingDone(int n, int clusters, double timeMs);

private:
    double m_eps = 1.0;
    int m_minPts = 5;
    double m_xi = 0.05;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute core distance for a point */
    double coreDist(int idx, const QVector<QVector<double>>& data,
                    const QVector<int>& neighbors) const;

    /** @brief Find epsilon-neighborhood */
    QVector<int> findNeighbors(int idx,
                               const QVector<QVector<double>>& data) const;

    /** @brief Update reachability for seeds */
    void updateSeeds(int idx, const QVector<int>& neighbors,
                     const QVector<QVector<double>>& data,
                     QVector<double>& reach, QVector<bool>& processed,
                     QVector<int>& ordering, int& orderPos);

    /** @brief Extract clusters via steepness evaluation */
    QVector<Cluster> extractClusters(const QVector<OpticsPoint>& ordering) const;
};
