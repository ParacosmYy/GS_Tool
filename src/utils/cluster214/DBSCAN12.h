/**
 * @file DBSCAN12.h
 * @brief DBSCAN密度聚类(自适应k距离肘部法则+边界点置信度评分) — DBSCAN with Adaptive Epsilon via K-Distance Elbow and Boundary Point Confidence Scoring
 *
 * 功能: 实现DBSCAN密度聚类算法，通过k-distance肘部法则自适应确定epsilon参数，
 *       支持边界点置信度评分，自动识别核心点、边界点和噪声点。
 *
 * 协作: KMeans20(核K均值) / OPTICS8(有序聚类) / GaussianMixture18(高斯混合)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief DBSCAN密度聚类(自适应epsilon+边界点置信度)
 */
class DBSCAN12 : public QObject {
    Q_OBJECT

public:
    /** @brief Point classification */
    enum PointType { Core = 0, Boundary = 1, Noise = 2 };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numSamples = 0;
        int numClusters = 0;
        int dimensions = 0;
        int coreCount = 0;
        int boundaryCount = 0;
        int noiseCount = 0;
        double epsilon = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit DBSCAN12(QObject *parent = nullptr);
    ~DBSCAN12() override;

    /** @brief Set clustering parameters: minPts and optional epsilon (0 = auto) */
    void setParameters(int minPts, double epsilon = 0.0);

    /** @brief Fit model to data */
    void fit(const QVector<QVector<double>>& data);

    /** @brief Get cluster labels (-1 = noise) */
    QVector<int> labels() const;

    /** @brief Get point type classification */
    QVector<PointType> pointTypes() const;

    /** @brief Get boundary confidence scores [0..1] */
    QVector<double> boundaryConfidence() const;

    /** @brief Compute k-distance graph for elbow detection */
    QVector<double> kDistanceGraph(int k) const;

    /** @brief Compute Euclidean distance between two points */
    double distance(const QVector<double>& a,
                    const QVector<double>& b) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void clusteringCompleted(int clusters, int core, int boundary, int noise, double timeMs);

private:
    int m_minPts = 5;
    double m_epsilon = 0.0;
    int m_dim = 0;
    int m_n = 0;

    QVector<QVector<double>> m_data;
    QVector<int> m_labels;
    QVector<PointType> m_types;
    QVector<double> m_confidence;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Find epsilon using k-distance elbow method */
    double autoEpsilon() const;

    /** @brief Find neighbors within epsilon radius */
    QVector<int> rangeQuery(int pointIdx) const;

    /** @brief Compute boundary point confidence score */
    double computeConfidence(int pointIdx, const QVector<int>& neighbors) const;

    /** @brief Expand cluster via BFS from seed point */
    void expandCluster(int pointIdx, int clusterId,
                       const QVector<QVector<int>>& neighborLists);
};
