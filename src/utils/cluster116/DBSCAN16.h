#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief DBSCAN16 - DBSCAN密度聚类第16代实现
 *
 * 基于密度的空间聚类算法，可发现任意形状的簇并识别噪声点，
 * 支持k-distance图自动参数选择及加速索引结构。
 */
class DBSCAN16 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalClusterOps = 0; double avgProcessingTimeMs = 0.0; };
    explicit DBSCAN16(QObject* parent = nullptr);
    Stats stats() const { return m_stats; }
    void resetStatistics();

    /**
     * @brief 执行DBSCAN聚类
     * @param dataPoints 输入数据点集合
     * @param epsilon 邻域半径
     * @param minPoints 核心点最小邻居数
     * @return 各点聚类标签（-1表示噪声）
     */
    QVector<int> fit(const QVector<QVector<double>>& dataPoints,
                     double epsilon, int minPoints);

    /**
     * @brief 使用k-distance图自动估计epsilon参数
     * @param dataPoints 数据点集合
     * @param k k近邻数（通常等于minPoints）
     * @return 建议的epsilon值
     */
    double estimateEpsilon(const QVector<QVector<double>>& dataPoints, int k);

    /**
     * @brief 查询指定点的epsilon邻域内所有点
     * @param pointIndex 中心点索引
     * @param dataPoints 数据点集合
     * @param epsilon 邻域半径
     * @return 邻域内的点索引集合
     */
    QVector<int> regionQuery(int pointIndex,
                             const QVector<QVector<double>>& dataPoints,
                             double epsilon);

    /**
     * @brief 获取聚类结果中的噪声点数量
     * @return 噪声点数量
     */
    int noiseCount() const;

signals:
    void clusteringCompleted(int clusterCount);

private:
    Stats m_stats; double m_timeSum = 0.0;
};
