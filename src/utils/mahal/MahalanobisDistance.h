/**
 * @file MahalanobisDistance.h
 * @brief 马氏距离 (Mahalanobis Distance) 计算
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 马氏距离度量样本与分布之间的距离，
 * 考虑了特征间的协方差结构。
 * 广泛用于异常检测、聚类和分类。
 */

#ifndef MAHALANOBISDISTANCE_H
#define MAHALANOBISDISTANCE_H

#include <QElapsedTimer>
#include <QObject>
#include <QVector>
#include <cmath>

/**
 * @class MahalanobisDistance
 * @brief 马氏距离计算引擎
 *
 * 支持单样本和批量样本的马氏距离计算。
 * 调用方需预先提供协方差矩阵的逆矩阵。
 */
class MahalanobisDistance : public QObject
{
    Q_OBJECT

public:
    /** @brief 运行时统计信息 */
    struct Stats {
        quint64 totalComputations = 0;  ///< 累计计算次数(含批量)
        double avgProcessingTimeMs = 0.0; ///< 平均单次处理耗时(ms)
    };

    /** @brief 构造马氏距离计算器 @param parent 父对象 */
    explicit MahalanobisDistance(QObject *parent = nullptr);

    /**
     * @brief 计算单个样本的马氏距离
     * @param x 样本向量(d 维)
     * @param mean 均值向量(d 维)
     * @param covInverse d x d 协方差逆矩阵
     * @return 马氏距离 sqrt((x-mean)^T * CovInv * (x-mean))
     *
     * 输入维度不一致时返回 -1.0。
     */
    double compute(const QVector<double> &x,
                   const QVector<double> &mean,
                   const QVector<QVector<double>> &covInverse);

    /**
     * @brief 批量计算多个样本的马氏距离
     * @param data n 个样本，每个 d 维
     * @param mean 均值向量(d 维)
     * @param covInverse d x d 协方差逆矩阵
     * @return n 个马氏距离值，失败返回空数组
     */
    QVector<double> batchCompute(const QVector<QVector<double>> &data,
                                 const QVector<double> &mean,
                                 const QVector<QVector<double>> &covInverse);

    /** @brief 获取运行时统计 */
    Stats stats() const { return m_stats; }

    /** @brief 重置所有统计计数器 */
    void resetStatistics();

signals:
    /** @brief 计算完成信号 @param count 本次计算的样本数 */
    void computationCompleted(int count);

private:
    /**
     * @brief 计算 diff^T * covInverse * diff
     * @param diff 差值向量(d 维)
     * @param covInverse d x d 协方差逆矩阵
     * @return 二次型值(开方前的马氏距离平方)
     */
    static double quadraticForm(const QVector<double> &diff,
                                const QVector<QVector<double>> &covInverse);

    Stats m_stats;          ///< 统计数据
    QElapsedTimer m_timer;  ///< 计时器
    double m_timeAccum = 0.0; ///< 累计耗时(ms)
};

#endif // MAHALANOBISDISTANCE_H
