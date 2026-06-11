/**
 * @file Agglomerative15.h
 * @brief 层次聚类(Ward连接与Lance-Williams递推公式构建内存高效树状图) — Agglomerative Clustering with Ward Linkage and Lance-Williams Recurrence for Memory-efficient Dendrogram Construction
 *
 * 功能: 实现层次聚类(Agglomerative clustering)，采用Ward连接(Ward linkage)
 *       与Lance-Williams递推公式(Lance-Williams recurrence)构建内存高效树状图(memory-efficient dendrogram)。
 *
 * 协作: KMeans31(K-means) / DBSCAN17(密度聚类) / GaussianMixture34(高斯混合)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 层次聚类(Ward连接与Lance-Williams递推公式构建内存高效树状图)
 */
class Agglomerative15 : public QObject {
    Q_OBJECT

public:
    /** @brief Dendrogram node */
    struct DendroNode {
        int left = -1;          // Left child index (or point id if leaf)
        int right = -1;         // Right child index
        double distance = 0.0;  // Merge distance
        int size = 1;           // Number of original points in this cluster
    };

    /** @brief Clustering result */
    struct ClusterResult {
        QVector<DendroNode> dendrogram;   // n-1 merge steps
        QVector<int> labels;              // Flat cluster assignment
        int numClusters = 0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numPoints = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Agglomerative15(QObject *parent = nullptr);
    ~Agglomerative15() override;

    void setNumClusters(int k);
    void setMaxPoints(int maxPts);

    /** @brief Fit agglomerative clustering with Ward linkage */
    ClusterResult fit(const QVector<QVector<double>>& data);

    /** @brief Cut dendrogram at a given distance threshold */
    QVector<int> cutAtDistance(double threshold) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void fitDone(int n, int k, double timeMs);

private:
    int m_k = 2;
    int m_maxPoints = 5000;
    Stats m_stats;
    double m_timeSum = 0.0;

    QVector<DendroNode> m_dendrogram;
    int m_n = 0;

    /** @brief Compute squared Euclidean distance between two points */
    double sqDist(const QVector<double>& a, const QVector<double>& b) const;

    /** @brief Compute Ward linkage merge distance via Lance-Williams */
    double wardDistance(double dAC, double dBC, double dAB,
                       int szA, int szB, int szC) const;

    /** @brief Find pair with minimum distance in condensed matrix */
    void findMinPair(const QVector<double>& dist, int n,
                     const QVector<int>& active,
                     int& outI, int& outJ) const;

    /** @brief Build flat labels from dendrogram by BFS cutting */
    QVector<int> buildLabels(int k) const;
};
