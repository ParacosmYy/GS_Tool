/**
 * @file BirchClustering9.h
 * @brief BIRCH聚类(增量CF树重平衡+多探测簇特征合并) — BIRCH Clustering with Incremental CF-Tree Rebalancing and Multi-Probe Cluster Feature Merging
 *
 * 功能: 实现BIRCH增量聚类算法，支持CF树动态重平衡、
 *       多探测簇特征合并和自适应阈值调节。
 *
 * 协作: Agglomerative9(层次聚合) / KMeans19(K均值) / DBSCAN8(密度聚类)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief BIRCH聚类(增量CF树重平衡+多探测簇特征合并)
 */
class BirchClustering9 : public QObject {
    Q_OBJECT

public:
    /** @brief Cluster Feature entry */
    struct CFEntry {
        int n = 0;                  // point count
        QVector<double> ls;         // linear sum
        double ss = 0.0;            // square sum
        int childIndex = -1;        // child node in CF-tree
        bool isLeaf = true;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numPoints = 0;
        int numClusters = 0;
        int treeHeight = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit BirchClustering9(QObject *parent = nullptr);
    ~BirchClustering9() override;

    void setThreshold(double t);
    void setBranchingFactor(int b);
    void setMaxClusters(int k);

    /** @brief Insert a single point into CF-tree */
    void insertPoint(const QVector<double>& point);

    /** @brief Bulk insert multiple points */
    void fit(const QVector<QVector<double>>& data);

    /** @brief Get cluster assignments via CF merging */
    QVector<int> getLabels() const;

    /** @brief Multi-probe merge: merge close CF entries */
    void multiProbeMerge(double mergeThreshold);

    /** @brief Compute CF centroid */
    QVector<double> cfCentroid(const CFEntry& cf) const;

    /** @brief Distance between two CF entries */
    double cfDistance(const CFEntry& a, const CFEntry& b) const;

    /** @brief Rebalance CF-tree after threshold change */
    void rebalanceTree();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void clusteringCompleted(int clusters, double timeMs);

private:
    double m_threshold = 0.5;
    int m_branching = 50;
    int m_maxClusters = 10;
    int m_dim = 0;

    QVector<CFEntry> m_entries;
    QVector<QVector<double>> m_rawPoints;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Euclidean distance */
    static double euclidean(const QVector<double>& a, const QVector<double>& b);

    /** @brief Find nearest CF entry to a point */
    int findNearest(const QVector<double>& point) const;

    /** @brief Absorb point into CF entry */
    void absorbPoint(CFEntry& cf, const QVector<double>& point);

    /** @brief Split an over-capacity entry */
    void splitEntry(int idx);
};
