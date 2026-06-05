/**
 * @file MeanShift.cpp
 * @brief Mean Shift聚类引擎实现 — 核密度估计模态搜索
 */

#include "utils/cluster8/MeanShift.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
MeanShift::MeanShift(QObject* parent)
    : QObject(parent)
    , m_bandwidth(1.0)
    , m_kernel(KernelType::Gaussian)
    , m_convergenceThreshold(1e-4)
    , m_maxIterations(300)
    , m_timeSum(0.0)
{
}

/** @brief 设置带宽 @param bandwidth 带宽(>0) */
void MeanShift::setBandwidth(double bandwidth)
{
    m_bandwidth = qMax(1e-10, bandwidth);
}

/** @brief 设置核函数类型 @param kernel 核类型 */
void MeanShift::setKernel(KernelType kernel)
{
    m_kernel = kernel;
}

/** @brief 设置收敛阈值 @param threshold 位移阈值 */
void MeanShift::setConvergenceThreshold(double threshold)
{
    m_convergenceThreshold = qMax(1e-12, threshold);
}

/** @brief 设置最大迭代次数 @param maxIter 最大迭代 */
void MeanShift::setMaxIterations(int maxIter)
{
    m_maxIterations = qMax(1, maxIter);
}

/** @brief 执行Mean Shift聚类 @param points 输入数据点 @return 聚类列表 */
QList<MeanShift::Cluster> MeanShift::cluster(const QList<Point>& points)
{
    QElapsedTimer timer;
    timer.start();

    int n = points.size();
    if (n == 0) return {};

    /* 第一步: 对每个点执行Mean Shift搜索其模态 */
    QList<Point> modes;
    modes.reserve(n);
    for (int i = 0; i < n; ++i) {
        Point mode = shiftPoint(points[i], points);
        modes.append(mode);
    }

    /* 第二步: 合并相近模态为聚类 */
    double mergeThreshold = m_bandwidth * 0.5;
    QList<Cluster> clusters = mergeModes(modes, points, mergeThreshold);

    /* 第三步: 计算每个聚类的密度 */
    for (auto& c : clusters) {
        c.density = kernelDensity(c.center, points);
    }

    /* 更新统计 */
    m_stats.totalClusterings++;
    m_stats.totalPointsProcessed += n;
    double elapsed = static_cast<double>(timer.nsecsElapsed()) / 1e6;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalClusterings);

    emit clusteringCompleted(clusters.size(), n);
    return clusters;
}

/** @brief Mean Shift迭代搜索模态 @param point 起始点 @param allPoints 数据点 @return 模态点 */
MeanShift::Point MeanShift::shiftPoint(const Point& point,
                                        const QList<Point>& allPoints) const
{
    Point current = point;
    int dims = point.size();
    double bwSq = m_bandwidth * m_bandwidth;

    for (int iter = 0; iter < m_maxIterations; ++iter) {
        Point numerator(dims, 0.0);
        double denominator = 0.0;

        /* 计算加权均值 */
        for (const auto& pt : allPoints) {
            double distSq = 0.0;
            for (int d = 0; d < dims; ++d) {
                double diff = pt[d] - current[d];
                distSq += diff * diff;
            }

            /* 带宽外不参与计算(高斯核理论上无截断,但加速可设阈值) */
            double weight = kernelValue(distSq / bwSq);
            if (weight < 1e-12) continue;

            for (int d = 0; d < dims; ++d) {
                numerator[d] += weight * pt[d];
            }
            denominator += weight;
        }

        if (denominator < 1e-15) break;

        /* 计算新位置 */
        Point shifted(dims);
        for (int d = 0; d < dims; ++d) {
            shifted[d] = numerator[d] / denominator;
        }

        /* 检查收敛 */
        double shiftDist = distance(current, shifted);
        current = shifted;
        if (shiftDist < m_convergenceThreshold) break;
    }

    return current;
}

/** @brief 核密度估计 @param point 目标点 @param allPoints 数据点 @return 密度值 */
double MeanShift::kernelDensity(const Point& point,
                                 const QList<Point>& allPoints) const
{
    int dims = point.size();
    double bwSq = m_bandwidth * m_bandwidth;
    double density = 0.0;
    int n = allPoints.size();

    for (const auto& pt : allPoints) {
        double distSq = 0.0;
        for (int d = 0; d < dims; ++d) {
            double diff = pt[d] - point[d];
            distSq += diff * diff;
        }
        density += kernelValue(distSq / bwSq);
    }

    /* 归一化 */
    double norm = n * qPow(m_bandwidth, dims)
                  * qPow(2.0 * M_PI, dims / 2.0);
    return (norm > 0) ? density / norm : 0.0;
}

/** @brief Silverman法则自动估计带宽 @param points 数据点 @return 建议带宽 */
double MeanShift::estimateBandwidth(const QList<Point>& points) const
{
    if (points.isEmpty()) return 1.0;

    int n = points.size();
    int dims = points[0].size();

    /* 计算每个维度的标准差 */
    QVector<double> stdDev(dims, 0.0);
    for (int d = 0; d < dims; ++d) {
        double mean = 0.0;
        for (const auto& pt : points) {
            if (d < pt.size()) mean += pt[d];
        }
        mean /= n;

        double variance = 0.0;
        for (const auto& pt : points) {
            if (d < pt.size()) {
                double diff = pt[d] - mean;
                variance += diff * diff;
            }
        }
        stdDev[d] = qSqrt(variance / n);
    }

    /* Silverman法则: h = sigma * (4 / ((d+2)*n))^(1/(d+4)) */
    double sigma = 0.0;
    for (double s : stdDev) sigma += s;
    sigma /= dims; /* 平均标准差 */

    double bandwidth = sigma * qPow(4.0 / ((dims + 2.0) * n),
                                     1.0 / (dims + 4.0));
    return qMax(1e-10, bandwidth);
}

/** @brief 欧氏距离 @param a @param b @return 距离 */
double MeanShift::distance(const Point& a, const Point& b)
{
    double distSq = 0.0;
    int dims = qMin(a.size(), b.size());
    for (int d = 0; d < dims; ++d) {
        double diff = a[d] - b[d];
        distSq += diff * diff;
    }
    return qSqrt(distSq);
}

/** @brief 核函数值 @param distSq 归一化距离平方 @return 核权重 */
double MeanShift::kernelValue(double distSq) const
{
    switch (m_kernel) {
    case KernelType::Gaussian:
        return qExp(-0.5 * distSq);
    case KernelType::Uniform:
        return (distSq <= 1.0) ? (1.0 - distSq) : 0.0; /* Epanechnikov */
    case KernelType::Cosine:
        return (distSq <= 1.0) ? qCos(M_PI_2 * qSqrt(distSq)) : 0.0;
    default:
        return qExp(-0.5 * distSq);
    }
}

/** @brief 合并相近模态为聚类 @param modes 模态列表 @param points 原始数据 @param mergeThreshold 合并阈值 @return 聚类列表 */
QList<MeanShift::Cluster> MeanShift::mergeModes(const QList<Point>& modes,
                                                  const QList<Point>& points,
                                                  double mergeThreshold) const
{
    int n = modes.size();
    QVector<int> labels(n, -1);
    QList<Point> uniqueModes;

    for (int i = 0; i < n; ++i) {
        if (labels[i] >= 0) continue;

        /* 寻找与当前模态相近的所有模态 */
        int label = uniqueModes.size();
        labels[i] = label;

        /* 计算组的均值作为聚类中心 */
        Point sum = modes[i];
        int count = 1;

        for (int j = i + 1; j < n; ++j) {
            if (labels[j] >= 0) continue;
            if (distance(modes[i], modes[j]) < mergeThreshold) {
                labels[j] = label;
                int dims = qMin(sum.size(), modes[j].size());
                for (int d = 0; d < dims; ++d) {
                    sum[d] += modes[j][d];
                }
                count++;
            }
        }

        Point center(sum.size());
        for (int d = 0; d < center.size(); ++d) {
            center[d] = sum[d] / count;
        }
        uniqueModes.append(center);
    }

    /* 构建聚类结果 */
    QList<Cluster> clusters;
    clusters.reserve(uniqueModes.size());
    for (int c = 0; c < uniqueModes.size(); ++c) {
        Cluster cluster;
        cluster.center = uniqueModes[c];
        for (int i = 0; i < n; ++i) {
            if (labels[i] == c) {
                cluster.memberIndices.append(i);
            }
        }
        clusters.append(cluster);
    }

    return clusters;
}

/** @brief 重置统计信息 */
void MeanShift::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
