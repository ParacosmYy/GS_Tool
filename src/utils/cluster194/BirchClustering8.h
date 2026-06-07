/**
 * @file BirchClustering8.h
 * @brief BIRCH聚类(增量CF树重平衡+离群缓冲+多分辨率) — BIRCH Clustering with Incremental CF-Tree Rebalancing, Outlier Buffer and Multi-Resolution
 *
 * 功能: 实现BIRCH聚类算法，支持增量CF树构建、
 *       自动重平衡、离群点缓冲区和多分辨率聚类。
 *
 * 协作: KMeans18(核K均值) / GaussianMixture15(高斯混合) / DBSCAN6(密度聚类)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief BIRCH聚类(CF树重平衡+离群缓冲)
 */
class BirchClustering8 : public QObject {
    Q_OBJECT

public:
    /** @brief Clustering Feature sub-cluster */
    struct CF {
        int n = 0;              // number of points
        QVector<double> ls;     // linear sum
        double ss = 0.0;        // square sum (scalar, per-dim aggregated)
    };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalRuns = 0;
        int numPoints = 0;
        int numLeaves = 0;
        int numOutliers = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit BirchClustering8(QObject *parent = nullptr);
    ~BirchClustering8() override;

    void setBranchFactor(int b);
    void setThreshold(double t);
    void setMaxLeafEntries(int m);
    void setClusters(int k);
    void setOutlierRatio(double ratio);

    /** @brief Build CF-tree incrementally from data points */
    void buildTree(const QVector<QVector<double>>& data);

    /** @brief Extract final clusters via k-means on leaf CFs */
    QVector<int> cluster(int k);

    /** @brief Insert a single point into the CF-tree */
    void insertPoint(const QVector<double>& point);

    /** @brief Get cluster centers */
    QVector<QVector<double>> centers() const;

    /** @brief Get outlier points buffered during build */
    QVector<QVector<double>> outliers() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void treeRebalanced(int leaves, int outliers);
    void clusteringCompleted(int k, double timeMs);

private:
    /** @brief Leaf entry: a CF sub-cluster */
    struct LeafEntry {
        CF cf;
        int id = -1;
    };

    /** @brief Non-leaf node with child pointers */
    struct TreeNode {
        CF cf;
        QVector<int> children;  // indices into m_nodes
        bool isLeaf = false;
        QVector<LeafEntry> entries; // valid when isLeaf
    };

    int m_branchFactor = 50;
    double m_threshold = 0.5;
    int m_maxLeafEntries = 20;
    int m_k = 3;
    double m_outlierRatio = 0.01;

    QVector<TreeNode> m_nodes;
    int m_root = -1;
    int m_dim = 0;

    QVector<QVector<double>> m_centers;
    QVector<QVector<double>> m_outlierBuf;
    QVector<int> m_labels;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Merge two CFs */
    static CF mergeCF(const CF& a, const CF& b);

    /** @brief Radius of a CF sub-cluster */
    static double cfRadius(const CF& cf);

    /** @brief Centroid of a CF */
    static QVector<double> cfCentroid(const CF& cf);

    /** @brief Distance between point and CF centroid */
    static double distToCF(const QVector<double>& pt, const CF& cf);

    /** @brief Find closest leaf entry to a point */
    int findClosest(const QVector<double>& pt, int nodeIdx) const;

    /** @brief Split an overfull leaf node */
    int splitLeaf(int nodeIdx);

    /** @brief Absorb outlier buffer back into tree */
    void absorbOutliers();
};
