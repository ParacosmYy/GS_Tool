/**
 * @file BirchClustering10.h
 * @brief BIRCH聚类(自适应阈值CF树+离群点周期重吸收) — BIRCH Clustering with Adaptive-Threshold CF-Tree and Outlier Buffer with Periodic Reabsorption
 *
 * 功能: 实现BIRCH聚类算法，使用自适应阈值CF特征树增量聚类，
 *       离群点缓冲区通过周期性重吸收机制提升聚类质量。
 *
 * 协作: Agglomerative10(层次凝聚) / KMeans21(K均值) / DBSCAN12(密度聚类)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief BIRCH聚类(自适应阈值CF树+离群点重吸收)
 */
class BirchClustering10 : public QObject {
    Q_OBJECT

public:
    /** @brief Clustering feature sub-cluster */
    struct CFEntry {
        int n = 0;
        QVector<double> ls;   // linear sum
        double ss = 0.0;      // squared sum (scalar)
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numPoints = 0;
        int numLeafEntries = 0;
        int numOutliers = 0;
        int numReabsorbed = 0;
        int treeDepth = 0;
        double threshold = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit BirchClustering10(QObject *parent = nullptr);
    ~BirchClustering10() override;

    /** @brief Set parameters: initial threshold, branching factor, outlier capacity */
    void setParameters(double threshold = 0.5, int branching = 50, int outlierCap = 200);

    /** @brief Insert a single data point into the CF-tree */
    void insertPoint(const QVector<double>& point);

    /** @brief Batch insert data points */
    void fit(const QVector<QVector<double>>& data);

    /** @brief Get cluster centroids from leaf entries */
    QVector<QVector<double>> centroids() const;

    /** @brief Assign points to nearest centroid */
    QVector<int> predict(const QVector<QVector<double>>& data) const;

    /** @brief Periodic reabsorption of outliers into the CF-tree */
    void reabsorbOutliers();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void clusteringCompleted(int clusters, int outliers, double timeMs);

private:
    struct CFNode {
        bool isLeaf = false;
        CFEntry cf;
        QVector<CFNode*> children;
        CFNode* parent = nullptr;
        ~CFNode();
    };

    double m_threshold = 0.5;
    int m_branching = 50;
    int m_outlierCap = 200;
    int m_dim = 0;

    CFNode* m_root = nullptr;
    QVector<QVector<double>> m_outlierBuf;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute radius of a CF entry */
    double cfRadius(const CFEntry& cf) const;

    /** @brief Merge src CF entry into dst */
    void cfMerge(CFEntry& dst, const CFEntry& src) const;

    /** @brief Create CFEntry from a single point */
    CFEntry pointToCF(const QVector<double>& point) const;

    /** @brief Find closest child to point among children */
    int closestChild(const QVector<CFNode*>& children, const QVector<double>& point) const;

    /** @brief Euclidean distance between two vectors */
    double euclidean(const QVector<double>& a, const QVector<double>& b) const;

    /** @brief Insert point into the CF-tree recursively */
    bool insertIntoTree(const QVector<double>& point);

    /** @brief Split an overflowing leaf node */
    void splitLeaf(CFNode* leaf);

    /** @brief Update CF entries up to root */
    void updateCFPath(CFNode* node);

    /** @brief Adaptively adjust threshold based on tree occupancy */
    void adaptThreshold();

    /** @brief Compute tree depth */
    int computeDepth(CFNode* node) const;

    /** @brief Collect leaf entries recursively */
    void collectLeaves(CFNode* node, QVector<CFEntry>& entries) const;

    /** @brief Recursive node cleanup */
    void clearNode(CFNode* node);
};
