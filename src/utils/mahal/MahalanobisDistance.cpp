/**
 * @file MahalanobisDistance.cpp
 * @brief 马氏距离计算实现
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/mahal/MahalanobisDistance.h"

#include <QtGlobal>

/**
 * @brief 构造函数
 * @param parent 父 QObject
 */
MahalanobisDistance::MahalanobisDistance(QObject *parent)
    : QObject(parent)
{
}

/**
 * @brief 计算二次型 diff^T * CovInv * diff
 *
 * 对 d 维向量执行矩阵-向量乘法: temp = CovInv * diff，
 * 然后做内积: result = diff^T * temp。
 *
 * @param diff 差值向量
 * @param covInverse 协方差逆矩阵
 * @return 二次型值
 */
double MahalanobisDistance::quadraticForm(const QVector<double> &diff,
                                          const QVector<QVector<double>> &covInverse)
{
    const int d = diff.size();
    // temp = covInverse * diff
    QVector<double> temp(d, 0.0);
    for (int i = 0; i < d; ++i) {
        for (int j = 0; j < d; ++j) {
            temp[i] += covInverse[i][j] * diff[j];
        }
    }
    // result = diff^T * temp
    double result = 0.0;
    for (int i = 0; i < d; ++i) {
        result += diff[i] * temp[i];
    }
    return result;
}

/**
 * @brief 计算单个样本的马氏距离
 *
 * 公式: D(x) = sqrt((x - mean)^T * Sigma^{-1} * (x - mean))
 *
 * @param x 样本向量
 * @param mean 均值向量
 * @param covInverse 协方差逆矩阵
 * @return 马氏距离，输入无效时返回 -1.0
 */
double MahalanobisDistance::compute(const QVector<double> &x,
                                    const QVector<double> &mean,
                                    const QVector<QVector<double>> &covInverse)
{
    m_timer.start();

    const int d = x.size();
    // 输入校验
    if (d < 1 || mean.size() != d || covInverse.size() != d) {
        emit computationCompleted(0);
        return -1.0;
    }
    for (int i = 0; i < d; ++i) {
        if (covInverse[i].size() != d) {
            emit computationCompleted(0);
            return -1.0;
        }
    }

    // 计算差值向量
    QVector<double> diff(d, 0.0);
    for (int i = 0; i < d; ++i) {
        diff[i] = x[i] - mean[i];
    }

    // 二次型求值
    double q = quadraticForm(diff, covInverse);

    // 更新统计
    double elapsed = static_cast<double>(m_timer.elapsed());
    m_stats.totalComputations++;
    m_timeAccum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeAccum / static_cast<double>(m_stats.totalComputations);

    emit computationCompleted(1);

    // 防止负值(数值误差)
    return (q >= 0.0) ? std::sqrt(q) : 0.0;
}

/**
 * @brief 批量计算多个样本的马氏距离
 *
 * 对每个样本重复: diff = data[i] - mean, D = sqrt(diff^T * CovInv * diff)。
 * 共享同一个均值和协方差逆矩阵，效率高于逐个调用 compute()。
 *
 * @param data n 个样本
 * @param mean 均值向量
 * @param covInverse 协方差逆矩阵
 * @return n 个马氏距离值
 */
QVector<double> MahalanobisDistance::batchCompute(
    const QVector<QVector<double>> &data,
    const QVector<double> &mean,
    const QVector<QVector<double>> &covInverse)
{
    m_timer.start();

    const int n = data.size();
    if (n == 0) {
        emit computationCompleted(0);
        return {};
    }
    const int d = mean.size();
    // 输入校验
    if (d < 1 || covInverse.size() != d) {
        emit computationCompleted(0);
        return {};
    }
    for (int i = 0; i < d; ++i) {
        if (covInverse[i].size() != d) {
            emit computationCompleted(0);
            return {};
        }
    }
    for (int i = 0; i < n; ++i) {
        if (data[i].size() != d) {
            emit computationCompleted(0);
            return {};
        }
    }

    QVector<double> results(n, 0.0);
    for (int i = 0; i < n; ++i) {
        // 差值向量
        QVector<double> diff(d, 0.0);
        for (int j = 0; j < d; ++j) {
            diff[j] = data[i][j] - mean[j];
        }
        double q = quadraticForm(diff, covInverse);
        results[i] = (q >= 0.0) ? std::sqrt(q) : 0.0;
    }

    // 更新统计
    double elapsed = static_cast<double>(m_timer.elapsed());
    m_stats.totalComputations += static_cast<quint64>(n);
    m_timeAccum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeAccum / static_cast<double>(m_stats.totalComputations);

    emit computationCompleted(n);
    return results;
}

/**
 * @brief 重置统计计数器
 */
void MahalanobisDistance::resetStatistics()
{
    m_stats.totalComputations = 0;
    m_stats.avgProcessingTimeMs = 0.0;
    m_timeAccum = 0.0;
}
