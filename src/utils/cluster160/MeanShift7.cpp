/**
 * @file MeanShift7.cpp
 * @brief MeanShift7 实现
 *
 * 实现均值漂移聚类：KNN自适应带宽计算、高斯/截断核均值漂移迭代、
 * 簇中心合并和标签分配。
 */

#include "utils/cluster160/MeanShift7.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>
#include <random>

/**
 * @brief 构造函数
 * @param parent 父QObject
 */
MeanShift7::MeanShift7(QObject* parent)
    : QObject(parent)
{
}

void MeanShift7::setK(int k)
{
    m_k = qMax(3, k);
}

void MeanShift7::setKernel(KernelType type)
{
    m_kernel = type;
}

void MeanShift7::setConvergenceThreshold(double threshold)
{
    m_convergenceThreshold = qMax(1e-10, threshold);
}

void MeanShift7::setMaxIterations(int maxIter)
{
    m_maxIterations = qMax(10, maxIter);
}

void MeanShift7::setMergeThreshold(double threshold)
{
    m_mergeThreshold = qMax(1e-10, threshold);
}

/**
 * @brief 计算两点欧氏距离
 */
double MeanShift7::euclidean(const QVector<double>& a, const QVector<double>& b) const
{
    double sum = 0.0;
    const int dim = qMin(a.size(), b.size());
    for (int i = 0; i < dim; ++i) {
        double diff = a[i] - b[i];
        sum += diff * diff;
    }
    return qSqrt(sum);
}

/**
 * @brief 计算单点的自适应带宽
 *
 * 找到K个最近邻，取平均距离作为带宽。
 */
double MeanShift7::adaptiveBandwidth(int pointIdx, const QVector<QVector<double>>& data) const
{
    const int n = data.size();
    QVector<QPair<double, int>> distances;
    distances.reserve(n - 1);

    for (int i = 0; i < n; ++i) {
        if (i == pointIdx) continue;
        distances.append({euclidean(data[pointIdx], data[i]), i});
    }

    std::sort(distances.begin(), distances.end());

    int knn = qMin(m_k, distances.size());
    double sumDist = 0.0;
    for (int i = 0; i < knn; ++i) {
        sumDist += distances[i].first;
    }

    /* 防止带宽为零(所有邻居重合) */
    return qMax(1e-10, sumDist / knn);
}

/**
 * @brief 单次均值漂移迭代
 *
 * 以point为中心，用核函数加权周围数据点，计算加权质心。
 */
QVector<double> MeanShift7::shiftPoint(const QVector<double>& point,
                                        const QVector<QVector<double>>& data,
                                        double bandwidth) const
{
    const int dim = point.size();
    QVector<double> shifted(dim, 0.0);
    double totalWeight = 0.0;

    for (int i = 0; i < data.size(); ++i) {
        double dist = euclidean(point, data[i]);
        double w = 0.0;

        if (m_kernel == KernelType::Gaussian) {
            /* 高斯核: exp(-0.5 * (d/h)^2) */
            double ratio = dist / bandwidth;
            w = qExp(-0.5 * ratio * ratio);
        } else {
            /* 截断核: 距离 < 带宽时权重为1 */
            w = (dist <= bandwidth) ? 1.0 : 0.0;
        }

        if (w > 1e-20) {
            for (int d = 0; d < dim; ++d) {
                shifted[d] += w * data[i][d];
            }
            totalWeight += w;
        }
    }

    if (totalWeight > 1e-20) {
        for (int d = 0; d < dim; ++d) {
            shifted[d] /= totalWeight;
        }
    } else {
        shifted = point;
    }

    return shifted;
}

/**
 * @brief 合并距离过近的簇中心
 */
void MeanShift7::mergeCenters()
{
    if (m_centers.size() <= 1) return;

    QVector<bool> merged(m_centers.size(), false);
    QVector<QVector<double>> newCenters;

    for (int i = 0; i < m_centers.size(); ++i) {
        if (merged[i]) continue;

        QVector<double> center = m_centers[i];
        int count = 1;

        for (int j = i + 1; j < m_centers.size(); ++j) {
            if (merged[j]) continue;
            if (euclidean(m_centers[i], m_centers[j]) < m_mergeThreshold) {
                /* 合并：累加坐标后取平均 */
                for (int d = 0; d < center.size(); ++d) {
                    center[d] += m_centers[j][d];
                }
                count++;
                merged[j] = true;
            }
        }

        for (int d = 0; d < center.size(); ++d) {
            center[d] /= count;
        }
        newCenters.append(center);
    }

    m_centers = newCenters;
}

/**
 * @brief 为每个点分配最近簇标签
 */
void MeanShift7::assignLabels(const QVector<QVector<double>>& data)
{
    const int n = data.size();
    m_labels.resize(n);

    for (int i = 0; i < n; ++i) {
        double bestDist = std::numeric_limits<double>::max();
        int bestCluster = 0;

        for (int c = 0; c < m_centers.size(); ++c) {
            double d = euclidean(data[i], m_centers[c]);
            if (d < bestDist) {
                bestDist = d;
                bestCluster = c;
            }
        }

        m_labels[i] = bestCluster;
    }
}

/**
 * @brief 执行均值漂移聚类
 *
 * 1) 为每个数据点计算KNN自适应带宽
 * 2) 迭代漂移每个点直到收敛
 * 3) 合并距离过近的收敛点作为簇中心
 * 4) 为每个点分配最近簇标签
 */
QVector<int> MeanShift7::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    const int n = data.size();
    if (n == 0) return QVector<int>();

    const int dim = data[0].size();

    /* 每个点作为初始位置开始漂移 */
    QVector<QVector<double>> shifted(n);
    for (int i = 0; i < n; ++i) {
        shifted[i] = data[i];
    }

    int totalIterations = 0;

    for (int i = 0; i < n; ++i) {
        double bw = adaptiveBandwidth(i, data);

        for (int iter = 0; iter < m_maxIterations; ++iter) {
            totalIterations++;
            QVector<double> newPoint = shiftPoint(shifted[i], data, bw);
            double shift = euclidean(shifted[i], newPoint);

            shifted[i] = newPoint;

            if (shift < m_convergenceThreshold) break;
        }
    }

    /* 收集所有收敛点作为候选中心 */
    m_centers = shifted;

    /* 合并距离过近的中心 */
    mergeCenters();

    /* 分配标签 */
    assignLabels(data);

    /* 统计 */
    m_stats.clusterCount = m_centers.size();
    m_stats.totalClusterOps++;
    m_stats.totalIterations += totalIterations;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalClusterOps > 0)
        ? m_timeSum / m_stats.totalClusterOps : 0.0;

    emit fitCompleted(m_centers.size(), totalIterations);
    return m_labels;
}

void MeanShift7::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
