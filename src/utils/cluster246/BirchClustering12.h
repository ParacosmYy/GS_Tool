/**
 * @file BirchClustering12.h
 * @brief BIRCH聚类(增量CF条目合并+基于直径的子树分裂自适应内存管理) — BIRCH Clustering with Incremental CF Entry Merging and Diameter-Based Subtree Splitting for Adaptive Memory Management
 *
 * 功能: 实现BIRCH聚类算法(Balanced Iterative Reducing and Clustering using
 *       Hierarchies)，使用聚类特征(CF)向量增量合并，通过基于直径的子树
 *       分裂策略自适应管理CF树内存。
 *
 * 协作: Agglomerative12(层次凝聚聚类) / KMeans25(K-means聚类) / GaussianMixture25(高斯混合模型)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief BIRCH聚类(增量CF合并+直径子树分裂)
 */
class BirchClustering12 : public QObject {
    Q_OBJECT

public:
    /** @brief Clustering Feature vector: (N, LS, SS) */
    struct CFEntry {
        int n = 0;
        QVector<double> linearSum;
        QVector<double> squareSum;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numSamples = 0;
        int numDimensions = 0;
        int numLeafEntries = 0;
        int numSplits = 0;
        int numMerges = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit BirchClustering12(QObject *parent = nullptr);
    ~BirchClustering12() override;

    /** @brief Set branching factor B for CF tree */
    void setBranchingFactor(int B);

    /** @brief Set diameter threshold T for leaf entry absorption */
    void setDiameterThreshold(double T);

    /** @brief Build CF tree and return cluster labels */
    QVector<int> fit(const QVector<QVector<double>>& data);

    /** @brief Get leaf-level CF entries */
    QVector<CFEntry> leafEntries() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void clusteringCompleted(int numClusters, int numSplits, double timeMs);

private:
    int m_branchingFactor = 50;
    double m_diameterThreshold = 0.5;

    /** @brief CF tree node (internal or leaf) */
    struct CFNode {
        bool isLeaf = false;
        CFEntry cf;
        QVector<int> children;  // Child node indices (-1 for leaf entries)
        QVector<QVector<double>> leafPoints; // Points in leaf
        int parent = -1;
    };

    QVector<CFNode> m_nodes;
    int m_root = -1;
    QVector<int> m_labels;
    int m_dim = 0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute centroid of a CF entry */
    QVector<double> cfCentroid(const CFEntry& cf) const;

    /** @brief Compute Euclidean diameter of a point set */
    double computeDiameter(const QVector<QVector<double>>& pts) const;

    /** @brief Merge CF entry src into dst */
    void mergeCF(CFEntry& dst, const CFEntry& src) const;

    /** @brief Create a new node */
    int createNode(bool isLeaf);

    /** @brief Insert a point into the CF tree */
    void insertPoint(const QVector<double>& point);

    /** @brief Find closest leaf entry index for a point */
    int findClosestLeaf(int nodeIdx, const QVector<double>& point) const;

    /** @brief Split a leaf node that exceeds capacity */
    int splitLeaf(int nodeIdx);

    /** @brief Update CF vectors upward to root */
    void updateCFPath(int nodeIdx);

    /** @brief Assign labels by final leaf clusters */
    void assignLabels(int n);
};
