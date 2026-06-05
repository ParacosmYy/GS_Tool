/**
 * @file SpectralClustering.h
 * @brief 谱聚类算法 (Spectral Clustering via Graph Laplacian)
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 基于图拉普拉斯矩阵的特征分解实现谱聚类。
 * 适用于非凸簇形状，优于 K-Means。
 * 内置幂迭代法求前 k 个特征向量，不依赖外部线性代数库。
 */

#ifndef SPECTRALCLUSTERING_H
#define SPECTRALCLUSTERING_H

#include <QElapsedTimer>
#include <QObject>
#include <QVector>
#include <algorithm>
#include <cmath>
#include <numeric>
#include <random>

/**
 * @class SpectralClustering
 * @brief 谱聚类引擎
 *
 * 输入亲和度矩阵和目标簇数 k，返回每个样本的簇标签。
 * 算法: 归一化拉普拉斯 → 幂迭代特征向量 → K-Means 分配。
 */
class SpectralClustering : public QObject
{
    Q_OBJECT

public:
    /** @brief 运行时统计信息 */
    struct Stats {
        quint64 totalClusterings = 0;    ///< 累计聚类调用次数
        double avgProcessingTimeMs = 0.0; ///< 平均单次处理耗时(ms)
    };

    /** @brief 构造谱聚类引擎 @param parent 父对象 */
    explicit SpectralClustering(QObject *parent = nullptr);

    /**
     * @brief 执行谱聚类
     * @param affinity n x n 亲和度矩阵(对称非负)
     * @param k 目标簇数(>=2)
     * @return 长度为 n 的簇标签数组(0 ~ k-1)，失败返回空
     */
    QVector<int> cluster(const QVector<QVector<double>> &affinity, int k);

    /** @brief 获取运行时统计 */
    Stats stats() const { return m_stats; }

    /** @brief 重置所有统计计数器 */
    void resetStatistics();

signals:
    /** @brief 聚类完成信号 @param k 簇数 @param n 样本数 */
    void clusteringCompleted(int k, int n);

private:
    /**
     * @brief 构建归一化拉普拉斯矩阵 L_norm = I - D^{-1/2} W D^{-1/2}
     * @param affinity 亲和度矩阵
     * @param diag D^{-1/2} 对角元素(输出)
     * @return 归一化拉普拉斯矩阵
     */
    static QVector<QVector<double>> buildNormLaplacian(
        const QVector<QVector<double>> &affinity, QVector<double> &diag);

    /**
     * @brief 幂迭代法求前 k 个最小特征向量
     * @param matrix 方阵
     * @param k 特征向量数
     * @param maxIter 最大迭代次数
     * @return k 个特征向量(每个长度 n)
     */
    static QVector<QVector<double>> powerEigenvectors(
        const QVector<QVector<double>> &matrix, int k, int maxIter = 200);

    /**
     * @brief 简单 K-Means 分配
     * @param features n x k 特征矩阵(每行一个样本)
     * @param k 簇数
     * @param maxIter 最大迭代次数
     * @return 簇标签
     */
    static QVector<int> kmeansAssign(
        const QVector<QVector<double>> &features, int k, int maxIter = 100);

    Stats m_stats;          ///< 统计数据
    QElapsedTimer m_timer;  ///< 计时器
    double m_timeAccum = 0.0; ///< 累计耗时(ms)
};

#endif // SPECTRALCLUSTERING_H
