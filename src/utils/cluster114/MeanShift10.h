#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief MeanShift10 - 均值漂移聚类第10代实现
 *
 * 基于核密度估计的均值漂移聚类算法，自动确定聚类数，
 * 支持高斯核/Epanechnikov核及多尺度带宽搜索。
 */
class MeanShift10 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalShiftOps = 0; double avgProcessingTimeMs = 0.0; };
    explicit MeanShift10(QObject* parent = nullptr);
    Stats stats() const { return m_stats; }
    void resetStatistics();

    /**
     * @brief 执行均值漂移聚类
     * @param dataPoints 输入数据点集合
     * @param bandwidth 核带宽参数
     * @return 各数据点的聚类标签
     */
    QVector<int> fit(const QVector<QVector<double>>& dataPoints, double bandwidth);

    /**
     * @brief 单点均值漂移迭代至收敛
     * @param point 起始点
     * @param dataPoints 所有数据点
     * @param bandwidth 核带宽
     * @param maxIterations 最大迭代次数
     * @return 收敛后的密度极大值点
     */
    QVector<double> shiftPoint(const QVector<double>& point,
                               const QVector<QVector<double>>& dataPoints,
                               double bandwidth, int maxIterations = 100);

    /**
     * @brief 自动估计最佳带宽（Silverman法则）
     * @param dataPoints 数据点集合
     * @return 建议的带宽值
     */
    double estimateBandwidth(const QVector<QVector<double>>& dataPoints);

    /**
     * @brief 设置核函数类型
     * @param kernelType 核类型 (gaussian/epanechnikov/uniform)
     */
    void setKernel(const QString& kernelType);

signals:
    void clusteringCompleted(int clusterCount);

private:
    Stats m_stats; double m_timeSum = 0.0;
};
