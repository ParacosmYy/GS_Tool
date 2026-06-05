/**
 * @file MeanShift5.cpp
 * @brief 均值漂移聚类算法实现 — 高斯核/平核 + 密度模态收敛
 */

#include "utils/cluster75/MeanShift5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
MeanShift5::MeanShift5(QObject* parent)
    : QObject(parent)
{
}

/** @brief 设置带宽参数 @param bandwidth 核函数半径 */
void MeanShift5::setBandwidth(double bandwidth)
{
    m_bandwidth = qMax(0.01, bandwidth);
}

/** @brief 设置核函数类型 @param kernelType gaussian或flat */
void MeanShift5::setKernel(const QString& kernelType)
{
    m_kernelType = kernelType;
}

/** @brief 重置统计数据 */
void MeanShift5::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
    m_modes.clear();
}

/**
 * @brief 高斯核函数
 * @param distSq 距离平方
 * @return 核权重
 */
static double gaussianKernel(double distSq, double bwSq)
{
    return qExp(-distSq / (2.0 * bwSq));
}

/**
 * @brief 平核函数
 * @param distSq 距离平方
 * @return 核权重(带内为1，带外为0)
 */
static double flatKernel(double distSq, double bwSq)
{
    return (distSq <= bwSq) ? 1.0 : 0.0;
}

/**
 * @brief 执行均值漂移聚类
 * @param data 输入数据矩阵
 * @return 每个样本的簇标签
 *
 * 每个样本沿密度梯度迭代移动，收敛后合并相近模态点。
 */
QVector<int> MeanShift5::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    QVector<int> labels(n, -1);
    m_modes.clear();

    if (n < 1) return labels;

    int dim = data[0].size();
    double bwSq = m_bandwidth * m_bandwidth;
    const int maxIter = 300;
    const double convergenceEps = 1e-4;

    /* 存储每个点偏移后的位置 */
    QVector<QVector<double>> shifted = data;

    /* 对每个点执行均值漂移 */
    for (int i = 0; i < n; ++i) {
        for (int iter = 0; iter < maxIter; ++iter) {
            QVector<double> newPoint(dim, 0.0);
            double totalWeight = 0.0;

            for (int j = 0; j < n; ++j) {
                double distSq = 0.0;
                for (int d = 0; d < dim; ++d) {
                    double diff = shifted[i][d] - data[j][d];
                    distSq += diff * diff;
                }

                double w = (m_kernelType == "flat")
                    ? flatKernel(distSq, bwSq)
                    : gaussianKernel(distSq, bwSq);

                if (w > 0.0) {
                    for (int d = 0; d < dim; ++d) {
                        newPoint[d] += w * data[j][d];
                    }
                    totalWeight += w;
                }
            }

            if (totalWeight > 0.0) {
                for (int d = 0; d < dim; ++d) {
                    newPoint[d] /= totalWeight;
                }
            }

            double moveSq = 0.0;
            for (int d = 0; d < dim; ++d) {
                double diff = newPoint[d] - shifted[i][d];
                moveSq += diff * diff;
            }

            shifted[i] = newPoint;

            if (moveSq < convergenceEps) {
                m_stats.totalShifts += iter + 1;
                break;
            }
        }
    }

    /* 合并相近模态点 */
    const double mergeThreshold = m_bandwidth * 0.5;
    QVector<QVector<double>> uniqueModes;

    for (int i = 0; i < n; ++i) {
        bool merged = false;
        for (int m = 0; m < uniqueModes.size(); ++m) {
            double distSq = 0.0;
            for (int d = 0; d < dim; ++d) {
                double diff = shifted[i][d] - uniqueModes[m][d];
                distSq += diff * diff;
            }
            if (qSqrt(distSq) < mergeThreshold) {
                labels[i] = m;
                merged = true;
                break;
            }
        }
        if (!merged) {
            labels[i] = uniqueModes.size();
            uniqueModes.append(shifted[i]);
        }
    }

    m_modes = uniqueModes;
    m_stats.totalClusters += uniqueModes.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / std::max(1, m_stats.totalClusters);

    emit clusteringCompleted(uniqueModes.size(), maxIter);
    return labels;
}

/** @brief 获取各簇的模态点 */
QVector<QVector<double>> MeanShift5::clusterModes() const
{
    return m_modes;
}
