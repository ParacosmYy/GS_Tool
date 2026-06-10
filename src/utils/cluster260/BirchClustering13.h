/**
 * @file BirchClustering13.h
 * @brief BIRCH增量聚类(CF条目增量合并与阈值自适应子簇分裂流式数据) — BIRCH Clustering with Incremental CF Entry Merging and Threshold-adaptive Subcluster Splitting for Streaming Data
 *
 * 功能: 实现BIRCH增量聚类(BIRCH clustering)，采用CF条目增量合并(CF entry
 *       incremental merging)和阈值自适应子簇分裂(threshold-adaptive
 *       subcluster splitting)处理流式数据(streaming data)。
 *
 * 协作: Agglomerative13(层次聚合聚类) / KMeans27(K均值) / DBSCAN15(密度聚类)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief BIRCH增量聚类(CF条目增量合并与阈值自适应子簇分裂流式数据)
 */
class BirchClustering13 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numPoints = 0;
        int numDimensions = 0;
        int numSubclusters = 0;
        int numSplits = 0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Clustering Feature (CF) entry */
    struct CFEntry {
        int n = 0;                          // number of points
        QVector<double> linearSum;          // LS
        QVector<double> squaredSum;         // SS
        QVector<double> centroid;           // cached centroid
    };

    /** @brief Subcluster node in the CF tree */
    struct Subcluster {
        CFEntry cf;
        bool isLeaf = true;
        int childIndex = -1;                // non-leaf child pointer
        QVector<int> leafEntries;           // leaf child indices
    };

    explicit BirchClustering13(QObject *parent = nullptr);
    ~BirchClustering13() override;

    /** @brief Set branching factor B and threshold T */
    void setParameters(int branchingFactor, double threshold);

    /** @brief Insert a single data point (streaming) */
    bool insertPoint(const QVector<double>& point);

    /** @brief Batch insert multiple points */
    bool fit(const QVector<QVector<double>>& data);

    /** @brief Get current subcluster centroids */
    QVector<QVector<double>> subclusterCentroids() const;

    /** @brief Get all CF entries */
    QVector<CFEntry> cfEntries() const;

    /** @brief Get point assignments to subclusters */
    QVector<int> labels() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void clusteringUpdated(int numSubclusters, int numSplits, double timeMs);
    void subclusterSplit(int oldId, int newId, double threshold);

private:
    int m_B = 50;           // branching factor
    double m_T = 0.5;       // radius threshold
    int m_dims = 0;

    QVector<Subcluster> m_subclusters;
    QVector<int> m_labels;
    int m_rootIndex = -1;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute Euclidean distance between two vectors */
    double euclidean(const QVector<double>& a, const QVector<double>& b) const;

    /** @brief Compute radius of a CF entry given a point */
    double cfRadius(const CFEntry& cf, const QVector<double>& point) const;

    /** @brief Merge two CF entries */
    CFEntry mergeCF(const CFEntry& a, const CFEntry& b) const;

    /** @brief Find closest subcluster to a point */
    int findClosest(int nodeIdx, const QVector<double>& point) const;

    /** @brief Insert point into CF tree recursively */
    bool insertIntoTree(int nodeIdx, const QVector<double>& point);

    /** @brief Split a subcluster when threshold exceeded */
    int splitSubcluster(int idx);

    /** @brief Update centroid from CF linear sum */
    void updateCentroid(CFEntry& cf) const;

    /** @brief Assign labels based on nearest subcluster */
    void assignLabels();
};
