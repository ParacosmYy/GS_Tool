#include "MeanShift8.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @file MeanShift8.cpp
 * @brief 均值漂移聚类器实现
 *
 * 基于核密度估计的非参数聚类算法:
 * 1. 对每个数据点计算核密度梯度方向
 * 2. 沿梯度方向漂移到密度更高的位置
 * 3. 收敛后的位置即为簇中心
 * 4. 合并距离足够近的收敛中心
 */

/**
 * @brief 构造函数，初始化默认核带宽
 * @param parent 父QObject对象指针
 */
MeanShift8::MeanShift8(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置核带宽参数
 * @param bandwidth 核函数的带宽参数，值越大簇越少
 */
void MeanShift8::setBandwidth(double bandwidth)
{
    m_bandwidth = qMax(0.01, bandwidth);
}

/**
 * @brief 设置核函数类型
 * @param kernel 核函数类型: gaussian/flat
 */
void MeanShift8::setKernel(const QString& kernel)
{
    if (kernel == "gaussian" || kernel == "flat") {
        m_kernel = kernel;
    }
}

/**
 * @brief 计算核函数值
 * @param distSq 距离的平方
 * @return 核函数权重值
 */
double MeanShift8::kernelValue(double distSq) const
{
    if (m_kernel == "gaussian") {
        return std::exp(-distSq / (2.0 * m_bandwidth * m_bandwidth));
    } else {
        // flat kernel: 距离小于带宽时为1，否则为0
        return (std::sqrt(distSq) <= m_bandwidth) ? 1.0 : 0.0;
    }
}

/**
 * @brief 拟合数据，执行均值漂移聚类
 *
 * 对每个数据点迭代执行均值漂移:
 * x_new = sum(K(||x-xi||/h) * xi) / sum(K(||x-xi||/h))
 * 直到收敛或达到最大迭代次数。
 *
 * @param data 输入数据矩阵，每行为一个数据点
 */
void MeanShift8::fit(const QVector<QVector<double>>& data)
{
    if (data.isEmpty()) return;

    QElapsedTimer timer;
    timer.start();

    const int n = data.size();
    const int dims = data[0].size();
    const int maxIter = 300;
    const double convergenceThreshold = 1e-3 * m_bandwidth;
    int totalIters = 0;

    // 拷贝数据作为初始漂移点
    QVector<QVector<double>> shifted = data;

    // 对每个点执行均值漂移
    for (int i = 0; i < n; ++i) {
        for (int iter = 0; iter < maxIter; ++iter) {
            totalIters++;

            QVector<double> numerator(dims, 0.0);
            double denominator = 0.0;

            // 计算加权均值
            for (int j = 0; j < n; ++j) {
                double distSq = 0.0;
                for (int d = 0; d < dims; ++d) {
                    distSq += (shifted[i][d] - data[j][d]) * (shifted[i][d] - data[j][d]);
                }

                const double weight = kernelValue(distSq);
                if (weight > 0.0) {
                    for (int d = 0; d < dims; ++d) {
                        numerator[d] += weight * data[j][d];
                    }
                    denominator += weight;
                }
            }

            // 更新漂移点位置
            if (denominator > 1e-12) {
                QVector<double> newPos(dims);
                for (int d = 0; d < dims; ++d) {
                    newPos[d] = numerator[d] / denominator;
                }

                // 检查收敛
                double shift = 0.0;
                for (int d = 0; d < dims; ++d) {
                    shift += (newPos[d] - shifted[i][d]) * (newPos[d] - shifted[i][d]);
                }
                shifted[i] = newPos;

                if (std::sqrt(shift) < convergenceThreshold) break;
            }
        }
    }

    // 合并距离足够近的收敛中心
    QVector<int> clusterLabels(n, -1);
    const double mergeThreshold = m_bandwidth * 0.5;
    int clusterCount = 0;

    for (int i = 0; i < n; ++i) {
        if (clusterLabels[i] >= 0) continue;

        clusterLabels[i] = clusterCount;
        for (int j = i + 1; j < n; ++j) {
            double distSq = 0.0;
            for (int d = 0; d < dims; ++d) {
                distSq += (shifted[i][d] - shifted[j][d]) * (shifted[i][d] - shifted[j][d]);
            }
            if (std::sqrt(distSq) < mergeThreshold) {
                clusterLabels[j] = clusterCount;
            }
        }
        clusterCount++;
    }

    // 更新统计信息
    m_stats.totalPoints += n;
    m_stats.totalIterations += totalIters;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, m_stats.totalPoints / n);

    emit clusteringCompleted(clusterCount);
}

/**
 * @brief 重置所有统计信息
 */
void MeanShift8::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
