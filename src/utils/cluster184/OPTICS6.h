/**
 * @file OPTICS6.h
 * @brief OPTICS排序聚类(可达距离图+陡升陡降Xi提取) — OPTICS Ordering with Reachability Plot Extraction, Cluster Extraction via Steep-Down/Up Areas and Xi Method
 *
 * 功能: 实现OPTICS聚类算法，支持核心距离计算、可达距离排序、
 *       陡升陡降区域检测和Xi法簇提取。
 *
 * 协作: DBSCAN10(密度聚类) / SubspaceCluster5(子空间聚类) / HierarchicalCluster7(层次聚类)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief OPTICS排序聚类器(可达距离图+Xi提取)
 */
class OPTICS6 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalRuns = 0;
        int numPoints = 0;
        int numClusters = 0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief 聚类结果 */
    struct ClusterResult {
        QVector<int> ordering;           // OPTICS ordering of point indices
        QVector<double> reachability;    // reachability distance per point
        QVector<double> coreDist;        // core distance per point
        QVector<int> labels;             // cluster labels (-1 = noise)
    };

    explicit OPTICS6(QObject *parent = nullptr);
    ~OPTICS6() override;

    void setEpsilon(double eps);
    void setMinPoints(int minPts);
    void setXi(double xi);

    /** @brief 执行OPTICS聚类 */
    ClusterResult fit(const QVector<QVector<double>>& data);

    /** @brief 从可达距离图用Xi法提取簇 */
    QVector<QVector<int>> extractClustersXi(const QVector<double>& reachability,
                                            const QVector<int>& ordering) const;

    /** @brief 计算核心距离 */
    double coreDistance(const QVector<QVector<double>>& data,
                       int idx, const QVector<int>& neighbors) const;

    /** @brief 计算两点欧氏距离 */
    double distance(const QVector<double>& a, const QVector<double>& b) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void clusteringCompleted(int numClusters, double totalTimeMs);

private:
    double m_epsilon = 1.0;
    int m_minPoints = 5;
    double m_xi = 0.05;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Find epsilon-neighborhood of point */
    QVector<int> findNeighbors(const QVector<QVector<double>>& data,
                               int idx) const;

    /** @brief Update reachability in seed list */
    void updateSeeds(QVector<QPair<double, int>>& seeds,
                     const QVector<QVector<double>>& data,
                     int pointIdx,
                     const QVector<bool>& processed,
                     QVector<double>& reachDist) const;

    /** @brief Detect steep-down area starting at index */
    QPair<int, int> findSteepDown(const QVector<double>& reach,
                                  int start) const;

    /** @brief Detect steep-up area starting at index */
    QPair<int, int> findSteepUp(const QVector<double>& reach,
                                int start) const;
};
