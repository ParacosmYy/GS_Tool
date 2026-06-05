#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief DBSCAN11 - 基于密度的空间聚类算法实现
 *
 * 支持任意距离度量的密度聚类，自动发现簇数量，
 * 可识别噪声点，适用于非凸形状数据集。
 */
class DBSCAN11 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalClusters = 0;
        int totalNoisePoints = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit DBSCAN11(QObject* parent = nullptr);

    /** @brief 执行DBSCAN聚类，返回每个点的簇标签(-1为噪声) */
    QVector<int> fit(const QVector<QVector<double>>& points, double epsilon, int minPts);

    /** @brief 使用预计算距离矩阵执行聚类 */
    QVector<int> fitFromDistance(const QVector<QVector<double>>& distMatrix, double epsilon, int minPts);

    /** @brief 设置自定义距离函数 */
    void setDistanceMetric(const QString& metric);

    /** @brief 获取指定簇的所有点索引 */
    QVector<int> getClusterPoints(int clusterId) const;

    /** @brief 获取核心点索引集合 */
    QVector<int> corePoints() const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void clusteringCompleted(int clusterCount, int noiseCount);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    QVector<int> m_labels;
    QVector<int> m_corePoints;
    QString m_metric;
};
