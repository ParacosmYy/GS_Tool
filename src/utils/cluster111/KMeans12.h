#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief KMeans12 - K-Means聚类算法第12代实现
 *
 * 支持多维数据的K-Means聚类，提供K-Means++初始化、
 * Mini-Batch变体、肘部法则评估及轮廓系数计算。
 */
class KMeans12 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalClusterOps = 0; double avgProcessingTimeMs = 0.0; };
    explicit KMeans12(QObject* parent = nullptr);
    Stats stats() const { return m_stats; }
    void resetStatistics();

    /**
     * @brief 执行K-Means聚类
     * @param dataPoints 输入数据点集合
     * @param k 聚类中心数量
     * @param maxIterations 最大迭代次数
     * @return 各数据点所属聚类标签
     */
    QVector<int> fit(const QVector<QVector<double>>& dataPoints, int k, int maxIterations = 100);

    /**
     * @brief 使用K-Means++策略初始化聚类中心
     * @param dataPoints 输入数据点集合
     * @param k 聚类中心数量
     * @return 初始化后的聚类中心
     */
    QVector<QVector<double>> initCentroidsKMeansPlusPlus(const QVector<QVector<double>>& dataPoints, int k);

    /**
     * @brief 计算轮廓系数评估聚类质量
     * @param dataPoints 数据点集合
     * @param labels 聚类标签
     * @return 轮廓系数值 [-1, 1]
     */
    double computeSilhouette(const QVector<QVector<double>>& dataPoints, const QVector<int>& labels);

    /**
     * @brief Mini-Batch K-Means变体，适用于大规模数据
     * @param dataPoints 输入数据点集合
     * @param k 聚类中心数量
     * @param batchSize 每批次样本数
     * @return 各数据点所属聚类标签
     */
    QVector<int> fitMiniBatch(const QVector<QVector<double>>& dataPoints, int k, int batchSize = 32);

signals:
    void clusterCompleted(int iterationCount);

private:
    Stats m_stats; double m_timeSum = 0.0;
};
