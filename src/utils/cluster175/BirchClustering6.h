/**
 * @file BirchClustering6.h
 * @brief BIRCH聚类(自适应阈值+子簇合并+CF树内存预算) — BIRCH Clustering with Adaptive Threshold, Subcluster Merging and CF-tree Memory Budgeting
 *
 * 功能: 实现BIRCH聚类算法，支持CF树构建、自适应半径阈值、
 *       子簇合并策略、内存预算控制和聚类特征向量统计。
 *
 * 协作: Agglomerative7(层次聚类) / KMedoids14(K-Medoids) / DBSCAN9(DBSCAN)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief BIRCH聚类器(CF树+自适应阈值)
 */
class BirchClustering6 : public QObject {
    Q_OBJECT

public:
    /** @brief 聚类特征(CF)向量 */
    struct CFEntry {
        int n = 0;                    ///< 点数
        QVector<double> ls;           ///< 线性和
        QVector<double> ss;           ///< 平方和
        QVector<double> centroid() const;
        double radius() const;
    };

    /** @brief CF树节点 */
    struct CFNode {
        QVector<CFEntry> entries;     ///< 非叶节点:子簇概要; 叶节点:数据概要
        QVector<CFNode*> children;    ///< 子节点指针(非叶节点)
        CFNode* parent = nullptr;
        bool isLeaf = true;
    };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalRuns = 0;
        int numClusters = 0;
        int numSamples = 0;
        int treeNodes = 0;
        double threshold = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit BirchClustering6(QObject *parent = nullptr);
    ~BirchClustering6() override;

    void setThreshold(double t);
    void setBranchingFactor(int b);
    void setNumClusters(int k);
    void setMemoryBudget(int maxNodes);

    QVector<int> fit(const QVector<QVector<double>>& data);

    /** @brief 获取叶节点子簇 */
    QVector<CFEntry> subclusters() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void clusteringCompleted(int numClusters, int numSubclusters);
    void treeRebuilt(int nodeCount);

private:
    double m_threshold = 0.5;
    int m_branching = 50;
    int m_numClusters = 3;
    int m_maxNodes = 10000;

    CFNode* m_root = nullptr;
    QVector<CFEntry> m_leafEntries;
    QVector<int> m_labels;
    int m_dims = 0;

    Stats m_stats;
    double m_timeSum = 0.0;

    void insertPoint(const QVector<double>& point);
    void rebuildTree();
    void mergeLeafEntries();
    void deleteTree(CFNode* node);
    static double euclideanDist(const QVector<double>& a,
                                const QVector<double>& b);
    static CFEntry mergeCF(const CFEntry& a, const CFEntry& b);
};
