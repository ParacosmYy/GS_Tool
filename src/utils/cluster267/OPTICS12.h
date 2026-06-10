/**
 * @file OPTICS12.h
 * @brief OPTICS聚类(Xi簇提取与可达距离图分析层次密度聚类) — OPTICS with Xi Cluster Extraction and Reachability Plot Analysis for Density-Based Hierarchical Clustering
 *
 * 功能: 实现OPTICS聚类算法，采用Xi簇提取(Xi cluster extraction)与可达距离图分析
 *       (reachability plot analysis)实现基于密度的层次聚类(density-based hierarchical clustering)。
 *
 * 协作: SubspaceCluster11(子空间聚类) / HierarchicalCluster13(层次聚类) / KMedoids21(K-中心点)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief OPTICS聚类(Xi簇提取与可达距离图分析层次密度聚类)
 */
class OPTICS12 : public QObject {
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

    /** @brief A cluster extracted by Xi method */
    struct Cluster {
        QVector<int> pointIndices;
        double startReach = 0.0;
        double endReach = 0.0;
    };

    explicit OPTICS12(QObject *parent = nullptr);
    ~OPTICS12() override;

    /** @brief Set epsilon neighborhood radius */
    void setEpsilon(double eps);

    /** @brief Set minimum points for core object */
    void setMinPts(int minPts);

    /** @brief Set Xi parameter for cluster extraction */
    void setXi(double xi);

    /** @brief Run OPTICS ordering and Xi extraction on 2D data */
    QVector<Cluster> fit(const QVector<QVector<double>>& data);

    /** @brief Get reachability distances in processing order */
    QVector<double> reachabilityPlot() const;

    /** @brief Get the OPTICS ordering of point indices */
    QVector<int> ordering() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void clusteringUpdated(int numClusters, int numPoints, double timeMs);

private:
    double m_epsilon = 1.0;
    int m_minPts = 5;
    double m_xi = 0.05;
    int m_n = 0;

    QVector<double> m_reachDist;
    QVector<double> m_coreDist;
    QVector<int> m_ordering;
    QVector<Cluster> m_clusters;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute pairwise distance */
    double dist(const QVector<QVector<double>>& data, int i, int j) const;

    /** @brief Find epsilon-neighborhood indices */
    QVector<int> neighbors(const QVector<QVector<double>>& data, int idx) const;

    /** @brief Compute core distance for a point */
    double coreDistance(const QVector<QVector<double>>& data, int idx) const;

    /** @brief Main OPTICS ordering loop using priority seed list */
    void computeOrdering(const QVector<QVector<double>>& data);

    /** @brief Extract clusters from reachability plot using Xi method */
    QVector<Cluster> extractXiClusters() const;

    /** @brief Find steep down area starting at position i */
    bool isSteepDown(int i, int& end) const;

    /** @brief Find steep up area starting at position i */
    bool isSteepUp(int i, int& end) const;
};
