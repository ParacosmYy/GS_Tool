/**
 * @file DBSCAN15.h
 * @brief HDBSCAN层次密度聚类(互达距离+最小生成树压缩层次提取) — HDBSCAN with Mutual Reachability Distance and MST Condensed Hierarchical Extraction for Variable Density Clustering
 *
 * 功能: 实现HDBSCAN层次密度聚类算法，采用互达距离(mutual reachability
 *       distance)构建最小生成树，通过压缩层次树(condensed tree)提取
 *       变密度聚类，自动确定聚类数量。
 *
 * 协作: KMeans26(核K均值) / OPTICS11(OPTICS聚类) / GaussianMixture27(高斯混合)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief HDBSCAN层次密度聚类(互达距离+最小生成树压缩层次提取)
 */
class DBSCAN15 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numPoints = 0;
        int numClusters = 0;
        int numNoise = 0;
        int numEdges = 0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Edge in mutual reachability graph */
    struct Edge {
        int u = -1;
        int v = -1;
        double weight = 0.0;
    };

    /** @brief Condensed tree node for hierarchy */
    struct CondensedNode {
        int parent = -1;
        int child = -1;
        double lambdaVal = 0.0;
        int childSize = 0;
    };

    explicit DBSCAN15(QObject *parent = nullptr);
    ~DBSCAN15() override;

    /** @brief Set minimum cluster size */
    void setMinClusterSize(int size);

    /** @brief Set minSamples for core distance */
    void setMinSamples(int samples);

    /** @brief Fit HDBSCAN to data */
    bool fit(const QVector<QVector<double>>& data);

    /** @brief Get cluster labels (-1 = noise) */
    QVector<int> labels() const;

    /** @brief Get cluster membership probabilities */
    QVector<double> probabilities() const;

    /** @brief Get condensed tree for visualization */
    QVector<CondensedNode> condensedTree() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void clusteringCompleted(int clusters, int noise, double timeMs);

private:
    int m_minClusterSize = 5;
    int m_minSamples = 5;
    int m_n = 0;
    int m_dims = 0;

    QVector<QVector<double>> m_data;
    QVector<double> m_coreDist;
    QVector<int> m_labels;
    QVector<double> m_probs;
    QVector<Edge> m_mstEdges;
    QVector<CondensedNode> m_condensed;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute pairwise distance */
    double distance(int i, int j) const;

    /** @brief Compute core distances for all points */
    void computeCoreDistances();

    /** @brief Compute mutual reachability distance */
    double mutualReachability(int i, int j) const;

    /** @brief Build MST via Prim's algorithm on mutual reachability */
    void buildMST();

    /** @brief Build condensed hierarchy from MST */
    void buildCondensedTree();

    /** @brief Extract stable clusters from condensed tree */
    void extractClusters();
};
