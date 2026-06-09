/**
 * @file OPTICS10.h
 * @brief OPTICS聚类(可达距离图提取+陡下降/上升区域自动簇检测) — OPTICS Clustering with Reachability Plot Extraction and Automatic Cluster Detection via Steep-Down/Up Area Detection
 *
 * 功能: 实现OPTICS排序点(OPTICS ordering)，通过核心距离(core distance)与可达距离
 *       (reachability distance)构建可达图(reachability plot)，利用陡下降区域
 *       (steep-down area)与陡上升区域(steep-up area)自动提取层次簇结构。
 *
 * 协作: SubspaceCluster9(子空间聚类) / DBSCAN7(密度聚类) / KMedoids19(K-中心点)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief OPTICS聚类(可达距离图提取+陡下降/上升区域自动簇检测)
 */
class OPTICS10 : public QObject {
    Q_OBJECT

public:
    /** @brief Cluster extracted from reachability plot */
    struct ClusterInfo {
        QVector<int> pointIndices;
        double startReachability = 0.0;
        double endReachability = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numPoints = 0;
        int numClusters = 0;
        int numDimensions = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit OPTICS10(QObject *parent = nullptr);
    ~OPTICS10() override;

    /** @brief Set epsilon neighborhood radius */
    void setEpsilon(double eps);

    /** @brief Set minimum points for core object */
    void setMinPoints(int minPts);

    /** @brief Set steepness threshold for area detection (0..1) */
    void setSteepThreshold(double t);

    /** @brief Run OPTICS on [n x d] data */
    void fit(const QVector<QVector<double>>& data);

    /** @brief Get reachability distances in OPTICS order */
    QVector<double> reachabilityPlot() const;

    /** @brief Get OPTICS ordering (original indices) */
    QVector<int> ordering() const;

    /** @brief Extract clusters via steep-down/up area detection */
    QVector<ClusterInfo> extractClusters() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void orderingCompleted(int n, double timeMs);
    void clusterExtracted(int numClusters, double timeMs);

private:
    double m_epsilon = 1.0;
    int m_minPts = 5;
    double m_steepThreshold = 0.05;

    QVector<QVector<double>> m_data;
    QVector<double> m_reachability;
    QVector<double> m_coreDist;
    QVector<int> m_ordering;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Euclidean distance between two points */
    double euclidean(const QVector<double>& a, const QVector<double>& b) const;

    /** @brief Compute core distance for point idx */
    double computeCoreDist(int idx, const QVector<int>& neighbors) const;

    /** @brief Find epsilon-neighborhood of point idx */
    QVector<int> findNeighbors(int idx) const;

    /** @brief Update reachability in seeds */
    void updateSeeds(int idx, QVector<double>& reach,
                     QVector<bool>& processed, QVector<int>& seeds);
};
