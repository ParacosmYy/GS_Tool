/**
 * @file OPTICS9.h
 * @brief OPTICS聚类(渐进可达性绘图+层次聚类提取) — OPTICS with Progressive Reachability Plotting and Hierarchical Cluster Extraction via Cluster-Ordering
 *
 * 功能: 实现OPTICS排序算法，生成渐进可达性图(reachability plot)，
 *       通过簇序(cluster-ordering)支持层次化聚类提取与密度分析。
 *
 * 协作: DBSCAN6(密度聚类) / SubspaceCluster8(子空间聚类) / KMedoids18(K中心点)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief OPTICS聚类(渐进可达性绘图+层次提取)
 */
class OPTICS9 : public QObject {
    Q_OBJECT

public:
    /** @brief Cluster-ordering entry */
    struct OrderEntry {
        int pointIndex = 0;
        double reachability = -1.0;   // undefined = -1
        double coreDistance = -1.0;
    };

    /** @brief Extracted cluster from reachability plot */
    struct Cluster {
        QVector<int> pointIndices;
        double startReach = 0.0;
        double endReach = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numPoints = 0;
        int numClustersExtracted = 0;
        int numCorePoints = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit OPTICS9(QObject *parent = nullptr);
    ~OPTICS9() override;

    /** @brief Set algorithm parameters */
    void setParameters(double epsilon, int minPoints);

    /** @brief Run OPTICS ordering on 2D point set */
    QVector<OrderEntry> fit(const QVector<QVector<double>>& data);

    /** @brief Extract clusters from reachability plot at given threshold */
    QVector<Cluster> extractClusters(double xiThreshold) const;

    /** @brief Get reachability values for plotting */
    QVector<double> reachabilityPlot() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void orderingCompleted(int n, int cores, double timeMs);
    void clustersExtracted(int count, double threshold);

private:
    double m_epsilon = 1.0;
    int m_minPoints = 5;

    QVector<OrderEntry> m_ordering;
    QVector<QVector<double>> m_data;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Euclidean distance between two points */
    double distance(const QVector<double>& a,
                    const QVector<double>& b) const;

    /** @brief Find neighbors within epsilon */
    QVector<int> rangeQuery(int idx) const;

    /** @brief Compute core distance for a point */
    double coreDistance(int idx, const QVector<int>& neighbors) const;

    /** @brief Update reachability for unprocessed neighbors */
    void updateSeeds(int idx, const QVector<int>& neighbors,
                     QVector<double>& reachDist,
                     QVector<bool>& processed,
                     QVector<int>& predecessor);
};
