/**
 * @file SubspaceCluster5.h
 * @brief 子空间聚类(PROCLUS投影追求+维度选择+簇精化) — Subspace Clustering via PROCLUS Projection Pursuit with Dimension Selection and Cluster Refinement
 *
 * 功能: 实现PROCLUS子空间聚类算法，支持投影追求维度选择、
 *       曼哈顿段距离度量、贪心初始化和迭代簇精化。
 *
 * 协作: HierarchicalCluster7(层次聚类) / KMedoids15(K-Medoids) / DBSCAN10(DBSCAN)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 子空间聚类器(PROCLUS投影追求)
 */
class SubspaceCluster5 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalRuns = 0;
        int numPoints = 0;
        int numClusters = 0;
        int totalDimensions = 0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief 子空间聚类结果 */
    struct ClusterResult {
        QVector<int> labels;
        QVector<QVector<int>> subspaceDims;
        QVector<QVector<double>> medoids;
    };

    explicit SubspaceCluster5(QObject *parent = nullptr);
    ~SubspaceCluster5() override;

    void setNumClusters(int k);
    void setAvgDimensions(int l);
    void setMaxIterations(int iter);

    /** @brief 执行PROCLUS子空间聚类 */
    ClusterResult fit(const QVector<QVector<double>>& data);

    /** @brief 计算曼哈顿段距离 */
    double manhattanSegmental(const QVector<double>& a,
                              const QVector<double>& b,
                              const QVector<int>& dims) const;

    /** @brief 投影追求维度选择 */
    QVector<int> selectDimensions(const QVector<QVector<double>>& data,
                                  int medoidIdx,
                                  const QVector<int>& candidateIdx) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void clusteringCompleted(int numClusters, double quality);

private:
    int m_numClusters = 3;
    int m_avgDimensions = 2;
    int m_maxIterations = 20;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Greedy medoid initialization */
    QVector<int> greedyInit(const QVector<QVector<double>>& data) const;

    /** @brief Assign points to nearest medoid using subspace */
    QVector<int> assignPoints(const QVector<QVector<double>>& data,
                              const QVector<int>& medoids,
                              const QVector<QVector<int>>& subspaces) const;

    /** @brief Compute cluster quality (negative cost) */
    double computeQuality(const QVector<QVector<double>>& data,
                          const QVector<int>& medoids,
                          const QVector<QVector<int>>& subspaces) const;
};
