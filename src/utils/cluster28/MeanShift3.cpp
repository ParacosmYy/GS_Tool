/**
 * @file MeanShift3.cpp
 * @brief 均值漂移增强实现 — 多核/自适应带宽/密度估计/层次模式
 */

#include "utils/cluster28/MeanShift3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <random>

/** @brief 构造函数 @param parent 父对象 */
MeanShift3::MeanShift3(QObject* parent)
    : QObject(parent)
{
}

/** @brief 设置带宽 @param h 核函数带宽 */
void MeanShift3::setBandwidth(double h)
{
    m_bandwidth = qMax(1e-6, h);
}

/** @brief 设置核函数类型 @param type 0=高斯 1=Epanechnikov 2=均匀 */
void MeanShift3::setKernelType(int type)
{
    m_kernelType = qBound(0, type, 2);
}

/** @brief 设置最大迭代次数 @param maxIter 最大迭代 */
void MeanShift3::setMaxIterations(int maxIter)
{
    m_maxIter = qMax(10, maxIter);
}

/** @brief 设置收敛阈值 @param tol 阈值 */
void MeanShift3::setConvergenceThreshold(double tol)
{
    m_tol = qMax(1e-10, tol);
}

/** @brief 执行均值漂移聚类 @param data 输入数据点集 @return 每个点的标签 */
QVector<int> MeanShift3::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    if (n == 0) {
        m_centers.clear();
        m_labels.clear();
        return m_labels;
    }

    int dim = data[0].size();
    m_labels.resize(n);

    /* 对每个点执行均值漂移直到收敛 */
    QVector<QVector<double>> shifted(n);
    for (int i = 0; i < n; ++i) {
        shifted[i] = data[i];
        for (int iter = 0; iter < m_maxIter; ++iter) {
            QVector<double> newPos = shiftPoint(shifted[i], data);
            double dist = 0.0;
            for (int d = 0; d < dim; ++d) {
                double diff = newPos[d] - shifted[i][d];
                dist += diff * diff;
            }
            shifted[i] = newPos;
            ++m_stats.totalShifts;
            if (qSqrt(dist) < m_tol) break;
        }
    }

    /* 合并距离小于带宽的收敛点作为聚类中心 */
    m_centers.clear();
    QVector<int> centerMap(n, -1);
    for (int i = 0; i < n; ++i) {
        int assigned = -1;
        for (int c = 0; c < m_centers.size(); ++c) {
            double dist = 0.0;
            for (int d = 0; d < dim; ++d) {
                double diff = shifted[i][d] - m_centers[c][d];
                dist += diff * diff;
            }
            if (qSqrt(dist) < m_bandwidth * 0.5) {
                assigned = c;
                break;
            }
        }
        if (assigned < 0) {
            assigned = m_centers.size();
            m_centers.append(shifted[i]);
        }
        centerMap[i] = assigned;
    }

    /* 分配标签 */
    for (int i = 0; i < n; ++i) {
        m_labels[i] = centerMap[i];
    }

    m_stats.totalPointsProcessed += n;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(qMax(1, m_stats.totalPointsProcessed));

    emit clusteringComplete(m_centers.size());
    return m_labels;
}

/** @brief 获取聚类中心 @return 聚类中心坐标集 */
QVector<QVector<double>> MeanShift3::clusterCenters() const
{
    return m_centers;
}

/** @brief 获取聚类数量 @return 聚类数量 */
int MeanShift3::clusterCount() const
{
    return m_centers.size();
}

/** @brief 对单个点执行均值漂移 @param point 当前点 @param data 全部数据 @return 漂移后的新位置 */
QVector<double> MeanShift3::shiftPoint(const QVector<double>& point,
                                        const QVector<QVector<double>>& data) const
{
    int dim = point.size();
    QVector<double> newPos(dim, 0.0);
    double totalWeight = 0.0;

    for (const auto& xi : data) {
        double distSq = 0.0;
        for (int d = 0; d < dim; ++d) {
            double diff = point[d] - xi[d];
            distSq += diff * diff;
        }
        double dist = qSqrt(distSq);
        double w = 0.0;

        if (m_kernelType == 0) {
            /* 高斯核 */
            w = gaussianKernel(dist);
        } else if (m_kernelType == 1) {
            /* Epanechnikov核 */
            double h = m_bandwidth;
            if (dist <= h) {
                double u = dist / h;
                w = 0.75 * (1.0 - u * u);
            }
        } else {
            /* 均匀核 */
            if (dist <= m_bandwidth) {
                w = 1.0;
            }
        }

        for (int d = 0; d < dim; ++d) {
            newPos[d] += w * xi[d];
        }
        totalWeight += w;
    }

    if (totalWeight > 1e-12) {
        for (int d = 0; d < dim; ++d) {
            newPos[d] /= totalWeight;
        }
    }
    return newPos;
}

/** @brief 高斯核函数 @param dist 距离 @return 核权重 */
double MeanShift3::gaussianKernel(double dist) const
{
    double h2 = m_bandwidth * m_bandwidth;
    return qExp(-0.5 * dist * dist / h2)
        / (m_bandwidth * qSqrt(2.0 * M_PI));
}

/**
 * @brief 自适应带宽估计 — 基于局部密度自动调整带宽
 * @param point 当前点
 * @param data 全部数据
 * @return 自适应带宽值
 *
 * 使用第k近邻距离作为局部密度估计，密度高的区域用较小带宽，
 * 密度低的区域用较大带宽，实现密度自适应的均值漂移。
 */
double MeanShift3::adaptiveBandwidth(const QVector<double>& point,
                                      const QVector<QVector<double>>& data) const
{
    int n = data.size();
    if (n == 0) return m_bandwidth;

    /* 计算当前点到所有其他点的距离 */
    QVector<double> distances(n);
    for (int i = 0; i < n; ++i) {
        double distSq = 0.0;
        int dim = qMin(point.size(), data[i].size());
        for (int d = 0; d < dim; ++d) {
            double diff = point[d] - data[i][d];
            distSq += diff * diff;
        }
        distances[i] = qSqrt(distSq);
    }

    /* 排序取第k近邻距离 */
    int k = qMax(1, static_cast<int>(qSqrt(static_cast<double>(n))));
    std::sort(distances.begin(), distances.end());
    double knnDist = distances[qMin(k, n - 1)];

    /* 自适应带宽 = 基础带宽与k近邻距离的加权平均 */
    return qMax(m_bandwidth * 0.5, 0.5 * (m_bandwidth + knnDist));
}

/**
 * @brief 估计点集的核密度
 * @param point 待估计点
 * @param data 参考数据集
 * @return 核密度估计值
 *
 * 使用当前配置的核函数类型和带宽对指定位置进行核密度估计。
 * 密度越高表示该位置附近的样本越密集。
 */
double MeanShift3::estimateDensity(const QVector<double>& point,
                                    const QVector<QVector<double>>& data) const
{
    int n = data.size();
    if (n == 0) return 0.0;

    int dim = point.size();
    double density = 0.0;
    double h = m_bandwidth;

    for (const auto& xi : data) {
        double distSq = 0.0;
        for (int d = 0; d < dim && d < xi.size(); ++d) {
            double diff = point[d] - xi[d];
            distSq += diff * diff;
        }
        double dist = qSqrt(distSq);

        if (m_kernelType == 0) {
            density += gaussianKernel(dist);
        } else if (m_kernelType == 1) {
            if (dist <= h) {
                double u = dist / h;
                density += 0.75 * (1.0 - u * u) / h;
            }
        } else {
            if (dist <= h) {
                density += 1.0 / h;
            }
        }
    }

    /* 归一化 */
    double volume = qPow(h, dim);
    density /= (static_cast<double>(n) * qMax(1e-12, volume));
    return density;
}

/** @brief 重置统计 */
void MeanShift3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
