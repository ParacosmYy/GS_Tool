/**
 * @file MiniBatchKMeans.h
 * @brief Mini-Batch K-Means聚类(Mini-Batch K-Means)
 */

#pragma once

#include <QObject>
#include <QVector>

/**
 * @class MiniBatchKMeans
 * @brief Mini-Batch K-Means — 大规模数据快速聚类
 *
 * 使用小批量随机梯度下降加速K-Means, 支持在线更新。
 * 适用于大规模数据集、流式数据聚类等场景。
 */
class MiniBatchKMeans : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalFitted = 0;      /**< 总拟合次数 */
        int totalPredictions = 0; /**< 总预测次数 */
        double avgProcessingTimeMs = 0.0; /**< 平均处理时间(ms) */
    };

    /** @brief 构造函数 */
    explicit MiniBatchKMeans(int k = 8, int batchSize = 100,
                               int maxIter = 100, QObject* parent = nullptr);

    /**
     * @brief 拟合模型
     * @param data 训练数据(每行一个样本)
     */
    void fit(const QVector<QVector<double>>& data);

    /**
     * @brief 在线更新(增量训练)
     * @param batch 新批次数据
     */
    void partialFit(const QVector<QVector<double>>& batch);

    /**
     * @brief 预测最近簇
     * @param point 数据点
     * @return 簇索引
     */
    int predict(const QVector<double>& point) const;

    /**
     * @brief 批量预测
     * @param data 数据集
     * @return 簇索引列表
     */
    QVector<int> predictBatch(const QVector<QVector<double>>& data) const;

    /**
     * @brief 计算惯性(样本到最近中心的距离总和)
     * @param data 数据集
     * @return 惯性值
     */
    double inertia(const QVector<QVector<double>>& data) const;

    /** @brief 获取簇中心 */
    QVector<QVector<double>> centroids() const;

    /** @brief 设置随机种子 */
    void setSeed(int seed);

    /** @brief 获取统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 拟合完成信号 */
    void fitted(int k, int iterations);

private:
    void initCentroids(const QVector<QVector<double>>& data);
    int nearestCentroid(const QVector<double>& point) const;
    static double euclidean(const QVector<double>& a, const QVector<double>& b);

    int m_k;
    int m_batchSize;
    int m_maxIter;
    int m_seed;
    int m_dims;
    QVector<QVector<double>> m_centroids;
    QVector<int> m_counts;

    Stats m_stats;
    double m_timeSum;
};
