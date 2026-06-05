/**
 * @file MeanShift2.cpp
 * @brief 均值漂移聚类实现 — 自适应带宽核密度估计 + 模式搜索
 */

#include "utils/cluster15/MeanShift2.h"

#include <QElapsedTimer>
#include <QtMath>

#include <algorithm>
#include <random>
#include <numeric>

/* ──────────────────── 构造/析构 ──────────────────── */

/** @brief 构造函数 @param parent 父对象 */
MeanShift2::MeanShift2(QObject* parent)
    : QObject(parent)
{
}

/** @brief 析构函数 */
MeanShift2::~MeanShift2() = default;

/* ──────────────────── 配置 ──────────────────── */

/** @brief 设置聚类参数 @param params 参数 */
void MeanShift2::setParameters(const Parameters& params)
{
    m_params = params;
}

/** @brief 获取当前参数 @return 参数 */
MeanShift2::Parameters MeanShift2::parameters() const
{
    return m_params;
}

/* ──────────────────── 1D聚类 ──────────────────── */

/** @brief 对1D数据进行均值漂移聚类 @param data 输入数据 @return 聚类结果 */
MeanShift2::ClusterResult MeanShift2::cluster1D(const QVector<double>& data)
{
    if (data.isEmpty()) return {};

    /* 转为多维格式(1D向量包装) */
    QVector<QVector<double>> multiData;
    multiData.reserve(data.size());
    for (double v : data) {
        multiData.append(QVector<double>{v});
    }
    return cluster(multiData);
}

/* ──────────────────── 多维聚类 ──────────────────── */

/** @brief 对多维数据进行均值漂移聚类 @param data 输入数据 @return 聚类结果 */
MeanShift2::ClusterResult MeanShift2::cluster(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    ClusterResult result;
    int n = data.size();
    if (n == 0) return result;

    int dims = data[0].size();

    /* 初始化模式点 = 每个数据点 */
    QVector<QVector<double>> modes = data;
    QVector<double> densities(n, 0.0);

    int totalIters = 0;

    /* 对每个点执行模式搜索 */
    for (int i = 0; i < n; ++i) {
        QVector<double> current = modes[i];
        double bw = m_params.bandwidth;

        for (int iter = 0; iter < m_params.maxIterations; ++iter) {
            /* 计算自适应带宽 */
            if (m_params.adaptiveBandwidth) {
                bw = adaptiveBW(current, data);
                bw = qMax(bw, 1e-6);
            }

            QVector<double> shifted = shiftPoint(current, data);
            if (shifted.isEmpty()) break;

            /* 检查收敛 */
            double shift = 0.0;
            for (int d = 0; d < dims; ++d) {
                double diff = shifted[d] - current[d];
                shift += diff * diff;
            }
            shift = qSqrt(shift);

            current = shifted;
            ++totalIters;

            if (shift < m_params.convergenceTol * bw) break;
        }

        modes[i] = current;
        densities[i] = kernelDensity(current, data);
    }

    /* 合并相近模式 */
    double mergeDist = m_params.bandwidth * m_params.clusterMergeDist;
    mergeModes(modes, densities, mergeDist);

    /* 分配标签 */
    result.modes = modes;
    result.densities = densities;
    result.numClusters = modes.size();
    result.iterations = totalIters;
    result.labels = assignLabels(data, modes, m_params.minDensity);

    /* 统计更新 */
    qint64 elapsed = timer.elapsed();
    m_timeSum += static_cast<double>(elapsed);
    m_stats.totalPointsProcessed += static_cast<quint64>(n);
    m_stats.totalClustersFound += static_cast<quint64>(result.numClusters);
    m_stats.totalShiftsComputed += static_cast<quint64>(totalIters);
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(qMax(m_stats.totalPointsProcessed, 1ULL));

    emit clusteringCompleted(result.numClusters, n);
    return result;
}

/* ──────────────────── 核密度估计 ──────────────────── */

/** @brief 核密度估计(单点) @param point 查询点 @param data 数据集 @return 密度值 */
double MeanShift2::kernelDensity(const QVector<double>& point,
                                 const QVector<QVector<double>>& data) const
{
    if (data.isEmpty()) return 0.0;

    int n = data.size();
    double bw = m_params.bandwidth;
    if (m_params.adaptiveBandwidth) {
        bw = adaptiveBW(point, data);
        bw = qMax(bw, 1e-6);
    }

    double sum = 0.0;
    for (int i = 0; i < n; ++i) {
        double d2 = distSq(point, data[i]);
        if (m_params.kernel == Gaussian) {
            sum += kernelGaussian(d2, bw);
        } else {
            sum += kernelEpanechnikov(d2, bw);
        }
    }
    return sum / (static_cast<double>(n) * qPow(bw, point.size()));
}

/* ──────────────────── 均值漂移向量 ──────────────────── */

/** @brief 单次均值漂移 @param point 当前点 @param data 数据集 @return 漂移后新位置 */
QVector<double> MeanShift2::shiftPoint(const QVector<double>& point,
                                       const QVector<QVector<double>>& data) const
{
    if (data.isEmpty()) return point;

    int dims = point.size();
    double bw = m_params.bandwidth;
    if (m_params.adaptiveBandwidth) {
        bw = adaptiveBW(point, data);
        bw = qMax(bw, 1e-6);
    }
    double bwSq = bw * bw;

    QVector<double> numerator(dims, 0.0);
    double denominator = 0.0;

    for (int i = 0; i < data.size(); ++i) {
        double d2 = distSq(point, data[i]);
        double weight = 0.0;

        if (m_params.kernel == Gaussian) {
            weight = qExp(-d2 / (2.0 * bwSq));
        } else {
            /* Epanechnikov: 导数为 (1 - d2/bwSq) 在 bwSq内 */
            if (d2 < bwSq) {
                weight = 1.0 - d2 / bwSq;
            }
        }

        if (weight > 0) {
            for (int d = 0; d < dims; ++d) {
                numerator[d] += weight * data[i][d];
            }
            denominator += weight;
        }
    }

    if (denominator < 1e-30) return point;

    QVector<double> shifted(dims);
    for (int d = 0; d < dims; ++d) {
        shifted[d] = numerator[d] / denominator;
    }
    return shifted;
}

/* ──────────────────── 核函数 ──────────────────── */

/** @brief 高斯核 @param distanceSq 距离平方 @param bw 带宽 @return 核值 */
double MeanShift2::kernelGaussian(double distanceSq, double bw) const
{
    return qExp(-distanceSq / (2.0 * bw * bw));
}

/** @brief Epanechnikov核 @param distanceSq 距离平方 @param bw 带宽 @return 核值 */
double MeanShift2::kernelEpanechnikov(double distanceSq, double bw) const
{
    double bwSq = bw * bw;
    if (distanceSq >= bwSq) return 0.0;
    double ratio = distanceSq / bwSq;
    return (1.0 - ratio);
}

/* ──────────────────── 辅助函数 ──────────────────── */

/** @brief 欧氏距离平方 @param a 点A @param b 点B @return 距离平方 */
double MeanShift2::distSq(const QVector<double>& a,
                          const QVector<double>& b) const
{
    double sum = 0.0;
    int dims = qMin(a.size(), b.size());
    for (int i = 0; i < dims; ++i) {
        double diff = a[i] - b[i];
        sum += diff * diff;
    }
    return sum;
}

/** @brief 计算自适应带宽(K近邻平均距离) @param point 查询点 @param data 数据集 @return 局部带宽 */
double MeanShift2::adaptiveBW(const QVector<double>& point,
                              const QVector<QVector<double>>& data) const
{
    int k = qMin(m_params.kNearest, data.size());
    if (k <= 0) return m_params.bandwidth;

    /* 计算到所有点的距离 */
    QVector<double> dists;
    dists.reserve(data.size());
    for (const auto& d : data) {
        dists.append(qSqrt(distSq(point, d)));
    }

    /* 排序取前K个 */
    std::partial_sort(dists.begin(), dists.begin() + k, dists.end());

    /* 平均K近邻距离作为局部带宽 */
    double sum = 0.0;
    for (int i = 0; i < k; ++i) {
        sum += dists[i];
    }
    double localBw = sum / static_cast<double>(k);
    return qMax(localBw, m_params.bandwidth * 0.1);
}

/** @brief 合并相近模式 @param modes 模式列表 @param densities 密度列表 @param mergeDist 合并距离 */
void MeanShift2::mergeModes(QVector<QVector<double>>& modes,
                            QVector<double>& densities,
                            double mergeDist) const
{
    if (modes.isEmpty()) return;

    QVector<bool> merged(modes.size(), false);
    QVector<QVector<double>> newModes;
    QVector<double> newDensities;

    for (int i = 0; i < modes.size(); ++i) {
        if (merged[i]) continue;

        QVector<double> mode = modes[i];
        double density = densities[i];
        int count = 1;

        /* 寻找与当前模式接近的其他模式 */
        for (int j = i + 1; j < modes.size(); ++j) {
            if (merged[j]) continue;
            double d = qSqrt(distSq(modes[i], modes[j]));
            if (d < mergeDist) {
                /* 加权平均(按密度加权) */
                double w = densities[j];
                for (int d2 = 0; d2 < mode.size(); ++d2) {
                    mode[d2] = (mode[d2] * density + modes[j][d2] * w)
                             / (density + w);
                }
                density = (density + w) / 2.0;
                merged[j] = true;
                ++count;
            }
        }

        newModes.append(mode);
        newDensities.append(density);
    }

    modes = newModes;
    densities = newDensities;
}

/** @brief 分配标签 @param data 数据集 @param modes 模式列表 @param minDensity 最小密度阈值 @return 标签列表 */
QVector<int> MeanShift2::assignLabels(const QVector<QVector<double>>& data,
                                      const QVector<QVector<double>>& modes,
                                      double minDensity) const
{
    QVector<int> labels(data.size(), -1);

    for (int i = 0; i < data.size(); ++i) {
        double minDist = 1e30;
        int bestMode = -1;

        for (int j = 0; j < modes.size(); ++j) {
            double d = distSq(data[i], modes[j]);
            if (d < minDist) {
                minDist = d;
                bestMode = j;
            }
        }

        /* 低密度点标记为噪声 */
        if (bestMode >= 0 && minDensity > 0) {
            double density = kernelDensity(data[i], data);
            if (density < minDensity) {
                labels[i] = -1;
                continue;
            }
        }

        labels[i] = bestMode;
    }

    return labels;
}

/* ──────────────────── 统计 ──────────────────── */

/** @brief 获取统计 @return 统计 */
MeanShift2::Stats MeanShift2::stats() const { return m_stats; }

/** @brief 重置统计 */
void MeanShift2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
