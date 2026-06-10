/**
 * @file BirchClustering14.h
 * @brief BIRCH聚类(自适应阈值与CF树重平衡的内存受限流式聚类维护) — BIRCH Clustering with Adaptive Threshold and CF Tree Rebalancing for Memory-constrained Streaming Cluster Maintenance
 *
 * 功能: 实现BIRCH聚类算法(Balanced Iterative Reducing and Clustering using Hierarchies)，
 *       采用自适应阈值(adaptive threshold)与CF树重平衡(CF tree rebalancing)
 *       实现内存受限流式聚类维护(memory-constrained streaming cluster maintenance)。
 *
 * 协作: Agglomerative14(层次凝聚聚类) / KMeans29(K均值) / DBSCAN16(密度聚类)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief BIRCH聚类(自适应阈值与CF树重平衡)
 */
class BirchClustering14 : public QObject {
    Q_OBJECT

public:
    /** @brief Clustering Feature (CF) sub-cluster entry */
    struct CFEntry {
        int n = 0;                       // Number of points
        QVector<double> linearSum;       // LS vector
        QVector<double> squareSum;       // SS vector
        int dim = 0;

        double radius() const;
        double centroid(int d) const;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numPoints = 0;
        int numSubClusters = 0;
        int numTreeNodes = 0;
        int numSplits = 0;
        int numRebalances = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit BirchClustering14(QObject *parent = nullptr);
    ~BirchClustering14() override;

    /** @brief Set dimensionality of data */
    void setDimensions(int d);

    /** @brief Set initial distance threshold for sub-cluster absorption */
    void setThreshold(double t);

    /** @brief Set max branching factor B for CF tree */
    void setBranchingFactor(int b);

    /** @brief Ingest a single data point into the CF tree */
    void insertPoint(const QVector<double>& point);

    /** @brief Batch ingest multiple points */
    void insertBatch(const QVector<QVector<double>>& data);

    /** @brief Get cluster labels using final clustering step */
    QVector<int> fit(int k);

    /** @brief Get all CF sub-cluster centroids */
    QVector<QVector<double>> subClusterCentroids() const;

    /** @brief Get current CF entries */
    QVector<CFEntry> cfEntries() const;

    /** @brief Trigger tree rebalance to reduce height */
    void rebalanceTree();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void clusteringDone(int k, int numSubClusters, int numSplits, double timeMs);
    void treeRebalanced(int oldHeight, int newHeight);

private:
    int m_dim = 2;
    double m_threshold = 0.5;
    int m_branchFactor = 50;
    double m_adaptiveThreshold = 0.5;

    /** @brief CF tree non-leaf node */
    struct CFNode {
        bool isLeaf = true;
        QVector<CFEntry> entries;     // Leaf: CF entries; Non-leaf: summaries
        QVector<int> children;        // Non-leaf: child node indices
        int parent = -1;
    };

    QVector<CFNode> m_nodes;
    int m_root = 0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Merge two CF entries */
    CFEntry mergeCF(const CFEntry& a, const CFEntry& b) const;

    /** @brief Compute Euclidean distance between a point and CF centroid */
    double distToCF(const QVector<double>& point, const CFEntry& cf) const;

    /** @brief Find closest leaf entry index for a point */
    int findClosestLeaf(const QVector<double>& point, int nodeIdx) const;

    /** @brief Insert point into a leaf node, returns true if split occurred */
    bool insertIntoLeaf(const QVector<double>& point, int leafIdx);

    /** @brief Split an overflowing leaf node */
    int splitLeaf(int leafIdx);

    /** @brief Propagate split upward */
    void propagateSplit(int nodeIdx, const CFEntry& newEntry, int newNodeIdx);

    /** @brief Adapt threshold based on current tree density */
    void adaptThreshold();

    /** @brief Update CF entry by adding a point */
    void addToCF(CFEntry& cf, const QVector<double>& point) const;

    /** @brief Compute tree height */
    int treeHeight() const;
};
