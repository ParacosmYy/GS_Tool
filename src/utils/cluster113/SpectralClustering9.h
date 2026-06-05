#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief SpectralClustering9 - 谱聚类第9代实现
 *
 * 基于图拉普拉斯矩阵的特征分解进行聚类，
 * 支持归一化/非归一化切割、K近邻图构建及自动聚类数选择。
 */
class SpectralClustering9 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalClusterOps = 0; double avgProcessingTimeMs = 0.0; };
    explicit SpectralClustering9(QObject* parent = nullptr);
    Stats stats() const { return m_stats; }
    void resetStatistics();

    /**
     * @brief 执行谱聚类
     * @param dataPoints 输入数据点集合
     * @param k 聚类数
     * @return 各数据点的聚类标签
     */
    QVector<int> fit(const QVector<QVector<double>>& dataPoints, int k);

    /**
     * @brief 构建K近邻亲和矩阵
     * @param dataPoints 数据点集合
     * @param kNeighbors 近邻数
     * @param sigma 高斯核宽度参数
     * @return 亲和矩阵
     */
    QVector<QVector<double>> buildKNNAffinity(const QVector<QVector<double>>& dataPoints,
                                               int kNeighbors, double sigma);

    /**
     * @brief 计算归一化图拉普拉斯矩阵
     * @param affinityMatrix 亲和矩阵
     * @return 归一化拉普拉斯矩阵
     */
    QVector<QVector<double>> normalizedLaplacian(const QVector<QVector<double>>& affinityMatrix);

    /**
     * @brief 自动选择最优聚类数（基于特征间隙）
     * @param dataPoints 数据点集合
     * @param maxK 最大候选聚类数
     * @return 推荐的聚类数
     */
    int autoSelectK(const QVector<QVector<double>>& dataPoints, int maxK = 10);

signals:
    void clusteringCompleted(int clusterCount);

private:
    Stats m_stats; double m_timeSum = 0.0;
};
