#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief BirchClustering11 - BIRCH聚类第11代实现
 *
 * 基于CF树（聚类特征树）的大规模数据增量聚类，
 * 支持分支因子/阈值调节及三阶段聚类流程。
 */
class BirchClustering11 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalInsertOps = 0; double avgProcessingTimeMs = 0.0; };
    explicit BirchClustering11(QObject* parent = nullptr);
    Stats stats() const { return m_stats; }
    void resetStatistics();

    /**
     * @brief 执行BIRCH聚类
     * @param dataPoints 输入数据点集合
     * @param branchingFactor CF树分支因子
     * @param threshold 聚类半径阈值
     * @return 各数据点的聚类标签
     */
    QVector<int> fit(const QVector<QVector<double>>& dataPoints,
                     int branchingFactor = 50, double threshold = 0.5);

    /**
     * @brief 增量插入新数据点到CF树
     * @param point 新数据点
     * @return 是否插入成功
     */
    bool insertPoint(const QVector<double>& point);

    /**
     * @brief 获取CF树的聚类特征摘要
     * @return 各子簇的 (点数, 线性和, 平方和) 三元组
     */
    QVector<QPair<int, QPair<QVector<double>, double>>> getClusteringFeatures() const;

    /**
     * @brief 合并相近的子簇
     * @param mergeThreshold 合并距离阈值
     * @return 合并后的子簇数量
     */
    int mergeSubclusters(double mergeThreshold);

signals:
    void clusteringCompleted(int clusterCount);

private:
    Stats m_stats; double m_timeSum = 0.0;
};
