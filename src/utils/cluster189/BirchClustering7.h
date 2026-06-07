/**
 * @file BirchClustering7.h
 * @brief BIRCH聚类(多探针CF树+子簇质量索引+内存有界增量聚类) — BIRCH with Multi-Probe CF-Tree Insertion, Subcluster Quality Index and Memory-Bounded Incremental Clustering
 *
 * 功能: 实现BIRCH聚类算法，支持多探针CF树插入、子簇质量索引(SQI)评估、
 *       内存有界增量聚类和可配置分支因子/阈值参数。
 *
 * 协作: Agglomerative8(层次聚类) / KMeans17(K均值) / DBSCAN10(密度聚类)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief BIRCH聚类器(多探针CF树+子簇质量索引)
 */
class BirchClustering7 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalRuns = 0;
        int numPoints = 0;
        int numSubclusters = 0;
        int treeHeight = 0;
        int rebuildCount = 0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Clustering Feature (CF) entry */
    struct CFEntry {
        int n = 0;
        QVector<double> ls;
        double ss = 0.0;
        double sqi = 0.0;
        int childIndex = -1;
    };

    explicit BirchClustering7(QObject *parent = nullptr);
    ~BirchClustering7() override;

    void setBranchFactor(int b);
    void setThreshold(double t);
    void setMaxMemory(int maxEntries);
    void setDimensions(int d);

    /** @brief Insert a single point and update CF-tree */
    void insertPoint(const QVector<double>& point);

    /** @brief Batch insert and cluster, return per-point labels */
    QVector<int> fit(const QVector<QVector<double>>& data);

    /** @brief Get subcluster centroids */
    QVector<QVector<double>> subclusterCentroids() const;

    /** @brief Get current leaf CF entries */
    QVector<CFEntry> leafEntries() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void clusteringCompleted(int subclusters, int rebuilds, double timeMs);

private:
    int m_branchFactor = 50;
    double m_threshold = 0.5;
    int m_maxMemory = 10000;
    int m_dimensions = 2;

    QVector<CFEntry> m_leaves;
    QVector<QVector<double>> m_centroids;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute centroid from CF entry */
    QVector<double> cfCentroid(const CFEntry& cf) const;

    /** @brief Euclidean distance between point and CF centroid */
    double cfDistance(const CFEntry& cf, const QVector<double>& point) const;

    /** @brief Subcluster Quality Index: radius-based measure */
    double computeSQI(const CFEntry& cf) const;

    /** @brief Merge two CF entries */
    CFEntry mergeCF(const CFEntry& a, const CFEntry& b) const;

    /** @brief Find closest leaf entry using multi-probe strategy */
    int multiProbeSearch(const QVector<double>& point) const;

    /** @brief Rebuild tree when memory bound exceeded */
    void rebuildTree();

    /** @brief Final clustering on leaf entries via agglomerative merge */
    QVector<int> finalizeClustering(int k);
};
