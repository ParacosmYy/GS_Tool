/**
 * @file KMeans17.h
 * @brief K-means聚类(Canopy预聚类初始化+Elkan三角不等式加速) — K-means Clustering with Canopy Pre-clustering Initialization and Elkan's Triangle Inequality Acceleration
 *
 * 功能: 实现K-means聚类算法，支持Canopy预聚类快速初始化、
 *       Elkan三角不等式避免冗余距离计算、多维度数据和收敛监控。
 *
 * 协作: GaussianMixture14(高斯混合) / DBSCAN10(密度聚类) / OPTICS6(有序聚类)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief K-means聚类器(Canopy预聚类+Elkan三角不等式加速)
 */
class KMeans17 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalRuns = 0;
        int numPoints = 0;
        int numClusters = 0;
        int iterationsUsed = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit KMeans17(QObject *parent = nullptr);
    ~KMeans17() override;

    void setK(int k);
    void setMaxIterations(int iter);
    void setTolerance(double tol);
    void setCanopyThreshold(double loose, double tight);

    /** @brief 执行K-means聚类，返回每个点的簇标签 */
    QVector<int> fit(const QVector<QVector<double>>& data);

    /** @brief 预测新样本的簇标签 */
    QVector<int> predict(const QVector<QVector<double>>& data) const;

    /** @brief 计算SSE(簇内平方和) */
    double sse() const { return m_sse; }

    /** @brief 获取聚类中心 */
    QVector<QVector<double>> centroids() const { return m_centroids; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void clusteringCompleted(int k, int iterations, double sse, double timeMs);

private:
    int m_k = 8;
    int m_maxIterations = 300;
    double m_tolerance = 1e-4;
    double m_canopyLoose = 2.0;
    double m_canopyTight = 1.0;

    QVector<QVector<double>> m_centroids;
    double m_sse = 0.0;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Euclidean distance between two points */
    double distance(const QVector<double>& a, const QVector<double>& b) const;

    /** @brief Canopy pre-clustering to find initial centers */
    QVector<QVector<double>> canopyInit(const QVector<QVector<double>>& data) const;

    /** @brief Elkan's algorithm: assign points using triangle inequality */
    QVector<int> elkanAssign(const QVector<QVector<double>>& data,
                             QVector<double>& lowerBounds,
                             QVector<double>& upperBounds,
                             QVector<int>& assignments) const;

    /** @brief Compute inter-centroid distances (half matrix) */
    QVector<QVector<double>> centroidDistances() const;

    /** @brief Update centroids from assignments */
    void updateCentroids(const QVector<QVector<double>>& data,
                         const QVector<int>& labels);
};
