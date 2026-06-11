/**
 * @file OPTICS14.h
 * @brief OPTICS聚类(epsilon有界可达性与陡降/陡升区域扫描提取变密度层次聚类) — OPTICS with Epsilon-bounded Reachability and Cluster Extraction via Steep-down/Up Area Scanning for Variable-density Hierarchical Clustering
 *
 * 功能: 实现OPTICS聚类算法(OPTICS clustering)，采用epsilon有界可达性(epsilon-bounded reachability)
 *       与陡降/陡升区域扫描(steep-down/up area scanning)提取变密度层次聚类(variable-density hierarchical clustering)。
 *
 * 协作: SubspaceCluster13(子空间聚类) / DBSCAN(密度聚类) / HierarchicalCluster15(层次聚类)
 */
#pragma once

#include <QObject>
#include <QVector>

class OPTICS14 : public QObject {
    Q_OBJECT

public:
    /** @brief Reachability-ordered point */
    struct PointInfo {
        int index = -1;
        double reachabilityDist = 1e300;
        double coreDist = 1e300;
        bool processed = false;
    };

    /** @brief Extracted cluster from reachability plot */
    struct Cluster {
        QVector<int> pointIndices;
        double minReachability = 0.0;
        double maxReachability = 0.0;
    };

    /** @brief Full OPTICS result */
    struct OPTICSResult {
        QVector<PointInfo> orderedPoints;
        QVector<Cluster> clusters;
        int numClusters = 0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalFits = 0;
        int numPoints = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit OPTICS14(QObject *parent = nullptr);
    ~OPTICS14() override;

    void setEpsilon(double eps);
    void setMinPoints(int minPts);

    /** @brief Run OPTICS on distance matrix */
    OPTICSResult fit(const QVector<QVector<double>>& distanceMatrix);

    /** @brief Extract clusters from reachability plot using steep area scanning */
    QVector<Cluster> extractClusters(const QVector<PointInfo>& ordered,
                                      double xi = 0.05) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void fitDone(int n, int k, double timeMs);

private:
    double m_epsilon = 1.0;
    int m_minPts = 5;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute core distance for a point */
    double coreDistance(const QVector<QVector<double>>& dist, int p) const;

    /** @brief Update reachability for neighbors of p */
    void update(const QVector<QVector<double>>& dist, int p,
                const QVector<int>& neighbors,
                QVector<PointInfo>& points,
                QVector<int>& seeds) const;

    /** @brief Find epsilon-neighborhood */
    QVector<int> regionQuery(const QVector<QVector<double>>& dist, int p) const;

    /** @brief Detect steep-down area starting at index */
    int steepDownArea(const QVector<PointInfo>& ordered,
                      int start, double xi) const;

    /** @brief Detect steep-up area starting at index */
    int steepUpArea(const QVector<PointInfo>& ordered,
                    int start, double xi) const;
};
