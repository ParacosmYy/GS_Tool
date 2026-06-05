#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief OPTICS9 - OPTICS聚类第9代实现
 *
 * 有序点识别聚类结构，生成可达距离图，
 * 支持任意形状簇发现及层次密度结构提取。
 */
class OPTICS9 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalClusterOps = 0; double avgProcessingTimeMs = 0.0; };
    explicit OPTICS9(QObject* parent = nullptr);
    Stats stats() const { return m_stats; }
    void resetStatistics();

    /**
     * @brief 执行OPTICS聚类分析
     * @param dataPoints 输入数据点集合
     * @param epsilon 邻域半径
     * @param minPoints 核心点最小邻居数
     * @return 有序点索引序列
     */
    QVector<int> fit(const QVector<QVector<double>>& dataPoints,
                     double epsilon, int minPoints);

    /**
     * @brief 获取可达距离序列
     * @return 各点的可达距离值
     */
    QVector<double> reachabilityDistances() const;

    /**
     * @brief 从可达距离图提取聚类（ξ方法）
     * @param xi 深度阈值参数
     * @return 各点的聚类标签
     */
    QVector<int> extractClusters(double xi = 0.05);

    /**
     * @brief 计算核心距离
     * @param pointIndex 点索引
     * @param dataPoints 数据点集合
     * @param epsilon 邻域半径
     * @param minPoints 最小邻居数
     * @return 核心距离值
     */
    double coreDistance(int pointIndex, const QVector<QVector<double>>& dataPoints,
                       double epsilon, int minPoints) const;

signals:
    void clusteringCompleted(int clusterCount);

private:
    Stats m_stats; double m_timeSum = 0.0;
};
