#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief DBSCAN密度聚类算法实现
 *
 * 基于密度的空间聚类，支持任意形状簇发现和噪声点检测。
 */
class DBSCAN12 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalPointsProcessed = 0;
        int totalClustersFound = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit DBSCAN12(QObject* parent = nullptr);

    /** @brief 执行聚类，返回每个点的簇编号(-1为噪声) */
    QVector<int> fit(const QVector<QVector<double>>& points, double eps, int minPts);

    /** @brief 获取聚类结果中的核心点索引 */
    QVector<int> coreIndices() const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void clusterCompleted(int clusterCount, int noiseCount);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    QVector<int> m_coreIndices;
};
