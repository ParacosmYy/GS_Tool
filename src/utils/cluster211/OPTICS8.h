/**
 * @file OPTICS8.h
 * @brief OPTICS聚类(可达距离图提取+陡降/陡升行走自动聚类) — OPTICS Clustering with Reachability Plot Extraction and Automatic Cluster Extraction via Steep Down/Up Walk
 *
 * 功能: 实现OPTICS排序算法，支持可达距离图生成、
 *       核心距离计算和陡降/陡升自动聚类提取。
 *
 * 协作: SubspaceCluster7(子空间聚类) / KMedoids17(K中心点) / DBSCAN1(密度聚类)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief OPTICS聚类(可达距离图+陡降/陡升自动提取)
 */
class OPTICS8 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numPoints = 0;
        int numClusters = 0;
        double epsilon = 0.0;
        int minPts = 0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief A cluster extracted from reachability plot */
    struct Cluster {
        QVector<int> pointIndices;
        double startReachability = 0.0;
        double endReachability = 0.0;
    };

    explicit OPTICS8(QObject *parent = nullptr);
    ~OPTICS8() override;

    /** @brief Set epsilon neighborhood radius and minPts */
    void setParameters(double epsilon, int minPts);

    /** @brief Run OPTICS ordering on data */
    void fit(const QVector<QVector<double>>& data);

    /** @brief Get reachability distances (ordered) */
    QVector<double> reachabilityPlot() const;

    /** @brief Get OPTICS ordering of point indices */
    QVector<int> ordering() const;

    /** @brief Extract clusters via steep down/up walk */
    QVector<Cluster> extractClusters(double xi = 0.05) const;

    /** @brief Compute core distance for a point */
    double coreDistance(int pointIdx,
                       const QVector<QVector<double>>& data) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void clusteringCompleted(int clusters, int ordered, double timeMs);

private:
    double m_epsilon = 1.0;
    int m_minPts = 5;
    int m_n = 0;

    QVector<double> m_reachability;
    QVector<double> m_coreDist;
    QVector<int> m_ordering;
    QVector<Cluster> m_clusters;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Euclidean distance between two points */
    static double euclidean(const QVector<double>& a,
                            const QVector<double>& b);

    /** @brief Find epsilon-neighborhood of point */
    QVector<QPair<double, int>> epsilonNeighbors(
        int pointIdx, const QVector<QVector<double>>& data) const;

    /** @brief Update reachability for seeds */
    void updateSeeds(int pointIdx,
                     const QVector<QVector<double>>& data,
                     QVector<double>& reachDist,
                     QVector<bool>& processed,
                     QVector<int>& predecessors,
                     QVector<QPair<double, int>>& seeds) const;

    /** @brief Detect steep down area in reachability plot */
    bool isSteepDown(int i, double xi) const;

    /** @brief Detect steep up area in reachability plot */
    bool isSteepUp(int i, double xi) const;
};
