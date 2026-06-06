/**
 * @file DBSCAN10.h
 * @brief DBSCAN聚类(HNSW加速邻域搜索+k距离肘部自适应eps) — DBSCAN with HNSW-accelerated Neighbor Search and Adaptive eps via k-Distance Elbow
 *
 * 功能: 实现DBSCAN密度聚类算法，支持HNSW近似近邻加速邻域查询、
 *       k-distance肘部法自适应eps参数选择和噪声点检测。
 *
 * 协作: KMeans16(K均值) / OPTICS6(OPTICS) / HDBSCAN7(HDBSCAN)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief DBSCAN聚类器(HNSW加速+k距离自适应eps)
 */
class DBSCAN10 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalRuns = 0;
        int numClusters = 0;
        int numCorePoints = 0;
        int numNoisePoints = 0;
        int numBorderPoints = 0;
        double estimatedEps = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit DBSCAN10(QObject *parent = nullptr);
    ~DBSCAN10() override;

    void setEps(double eps);
    void setMinPoints(int minPts);
    void setAutoEps(bool enabled);
    void setKDistanceK(int k);

    /** @brief 执行DBSCAN聚类，返回样本标签(-1为噪声) */
    QVector<int> fit(const QVector<QVector<double>>& data);

    /** @brief k-distance图数据(用于肘部法) */
    QVector<double> kDistanceGraph(const QVector<QVector<double>>& data, int k) const;

    /** @brief 自适应选择eps(肘部法) */
    double estimateEps(const QVector<QVector<double>>& data) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void clusteringCompleted(int numClusters, int numNoise, double eps);

private:
    double m_eps = 0.5;
    int m_minPts = 5;
    bool m_autoEps = false;
    int m_kDistK = 5;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief HNSW layer node */
    struct HNSWNode {
        int id = 0;
        int level = 0;
        QVector<int> neighbors; ///< Connections per layer
    };

    /** @brief Simple HNSW index for approximate neighbor search */
    QVector<QVector<int>> m_neighborCache;

    /** @brief Euclidean distance */
    double distance(const QVector<double>& a, const QVector<double>& b) const;

    /** @brief Build HNSW neighbor cache */
    void buildHNSWCache(const QVector<QVector<double>>& data);

    /** @brief Range query returning neighbor indices */
    QVector<int> rangeQuery(const QVector<QVector<double>>& data, int idx) const;

    /** @brief Expand cluster from seed point */
    void expandCluster(const QVector<QVector<double>>& data,
                        int idx, int clusterId,
                        QVector<int>& labels, QVector<bool>& visited);
};
