/**
 * @file BirchClustering11.h
 * @brief BIRCH聚类(自适应阈值+LRU子树淘汰的内存受限CF树) — BIRCH Clustering with Adaptive Threshold and Memory-Bounded CF-Tree Using LRU Subtree Eviction
 *
 * 功能: 实现BIRCH聚类算法(Balanced Iterative Reducing and Clustering using Hierarchies)，
 *       采用自适应阈值(adaptive threshold)动态调整CF树分裂参数，并使用LRU子树淘汰
 *       (least recently used subtree eviction)策略实现内存受限的CF树维护。
 *
 * 协作: Agglomerative11(层次凝聚聚类) / KMeans23(K均值) / DBSCAN13(密度聚类)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief BIRCH聚类(自适应阈值+LRU子树淘汰的内存受限CF树)
 */
class BirchClustering11 : public QObject {
    Q_OBJECT

public:
    /** @brief Clustering feature (CF) sub-vector */
    struct CFEntry {
        int n = 0;
        QVector<double> ls;   // linear sum
        double ss = 0.0;      // square sum
    };

    /** @brief Cluster assignment result */
    struct Assignment {
        int clusterId = -1;
        double distance = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numPoints = 0;
        int numDimensions = 0;
        int numLeafEntries = 0;
        int numEvictions = 0;
        double threshold = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit BirchClustering11(QObject *parent = nullptr);
    ~BirchClustering11() override;

    /** @brief Set initial radius threshold for CF tree */
    void setThreshold(double t);

    /** @brief Set maximum memory (max leaf entries before eviction) */
    void setMaxMemory(int maxEntries);

    /** @brief Set branching factor B for CF tree */
    void setBranchingFactor(int b);

    /** @brief Build CF tree from data [n x d] */
    bool fit(const QVector<QVector<double>>& data, int numClusters);

    /** @brief Get cluster assignments */
    QVector<Assignment> assignments() const;

    /** @brief Get leaf-level CF entries */
    QVector<CFEntry> leafEntries() const;

    /** @brief Get cluster centroids */
    QVector<QVector<double>> centroids() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void pointInserted(int index, int leafId);
    void treeRebuilt(int numLeaves, double newThreshold);
    void fitCompleted(int clusters, int leaves, double timeMs);

private:
    /** @brief CF tree leaf node */
    struct LeafNode {
        QVector<CFEntry> entries;
        int lastAccess = 0;
        LeafNode* next = nullptr;
    };

    /** @brief CF tree non-leaf node */
    struct InternalNode {
        QVector<CFEntry> entries;
        QVector<void*> children;
        int lastAccess = 0;
    };

    double m_threshold = 0.5;
    int m_branchingFactor = 50;
    int m_maxMemory = 1000;
    int m_accessCounter = 0;

    QVector<CFEntry> m_leafEntries;
    QVector<Assignment> m_assignments;
    QVector<QVector<double>> m_centroids;
    QVector<QVector<double>> m_data;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Merge two CF entries */
    CFEntry mergeCF(const CFEntry& a, const CFEntry& b) const;

    /** @brief Compute radius of CF entry */
    double cfRadius(const CFEntry& cf) const;

    /** @brief Compute centroid of CF entry */
    QVector<double> cfCentroid(const CFEntry& cf) const;

    /** @brief Euclidean distance between two points */
    double euclideanDist(const QVector<double>& a, const QVector<double>& b) const;

    /** @brief Find closest leaf entry to a point */
    int findClosestEntry(const QVector<double>& point) const;

    /** @brief Adaptively increase threshold */
    double adaptThreshold();

    /** @brief Evict LRU entries when memory bound exceeded */
    void evictLRU();

    /** @brief Rebuild CF tree with new threshold */
    void rebuildTree();

    /** @brief Assign points to nearest centroid */
    void assignPoints(const QVector<QVector<double>>& ctrs);
};
