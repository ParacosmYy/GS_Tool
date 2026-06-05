/**
 * @file HierarchicalDensity.cpp
 * @brief 层次密度聚类实现 — 密度峰值/决策图/中心选择/Halo
 */

#include "utils/cluster21/HierarchicalDensity.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/** @brief 构造函数 @param parent 父对象 */
HierarchicalDensity::HierarchicalDensity(QObject* parent)
    : QObject(parent)
    , m_method(DensityMethod::GaussianKernel)
    , m_cutoffRatio(0.2)
    , m_sigma(1.0)
    , m_dc(0.0)
{
}

/** @brief 设置密度计算方法 @param method 方法 */
void HierarchicalDensity::setDensityMethod(DensityMethod method)
{
    m_method = method;
}

/** @brief 设置截断半径比例 @param ratio 比例 */
void HierarchicalDensity::setCutoffRatio(double ratio)
{
    m_cutoffRatio = qBound(0.01, ratio, 1.0);
}

/** @brief 设置高斯核带宽 @param sigma 带宽 */
void HierarchicalDensity::setGaussianSigma(double sigma)
{
    m_sigma = qMax(0.001, sigma);
}

/** @brief 执行聚类 @param data 二维数据 @return 聚类结果 */
QList<HierarchicalDensity::ClusterPoint> HierarchicalDensity::cluster(
    const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    if (n < 2) {
        m_results.clear();
        return m_results;
    }

    /* 计算两两距离 */
    computePairwiseDistances(data);

    /* 计算局部密度 */
    QVector<double> density = computeDensity(n);

    /* 计算delta(到更高密度点的最短距离) */
    QVector<double> delta = computeDelta(n, density);

    /* 选择聚类中心 */
    QList<int> centers = selectCenters(n, density, delta);
    emit centersSelected(centers);

    /* 分配剩余点到最近中心 */
    assignClusters(n, centers, density);

    /* Halo噪声标记 */
    assignHalo(n);

    /* 更新统计 */
    double elapsed = timer.elapsed();
    m_stats.totalClusterOps++;
    m_stats.totalPointsProcessed += n;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalClusterOps);

    int numClusters = 0;
    for (const auto& pt : m_results) {
        if (pt.isCenter) numClusters++;
    }
    if (numClusters > m_stats.maxClusters) m_stats.maxClusters = numClusters;

    emit clusteringComplete(numClusters, n);
    return m_results;
}

/** @brief 获取决策图数据 @return (密度, delta) */
QPair<QVector<double>, QVector<double>> HierarchicalDensity::decisionGraph() const
{
    int n = m_results.size();
    QVector<double> density(n), delta(n);
    for (int i = 0; i < n; ++i) {
        density[i] = m_results[i].density;
        delta[i] = m_results[i].delta;
    }
    return {density, delta};
}

/** @brief 获取聚类成员 @param clusterId 聚类ID @return 点索引列表 */
QList<int> HierarchicalDensity::clusterMembers(int clusterId) const
{
    QList<int> members;
    for (const auto& pt : m_results) {
        if (pt.clusterId == clusterId) members.append(pt.index);
    }
    return members;
}

/** @brief 重置统计 */
void HierarchicalDensity::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/** @brief 计算两两距离矩阵 @param data 数据 */
void HierarchicalDensity::computePairwiseDistances(
    const QVector<QVector<double>>& data)
{
    int n = data.size();
    m_distMatrix.resize(n);
    for (int i = 0; i < n; ++i) {
        m_distMatrix[i].resize(n);
        m_distMatrix[i][i] = 0.0;
    }

    /* 收集所有距离用于确定截断距离 */
    QVector<double> allDists;
    allDists.reserve(n * (n - 1) / 2);

    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            double dist = 0.0;
            int dim = qMin(data[i].size(), data[j].size());
            for (int d = 0; d < dim; ++d) {
                double diff = data[i][d] - data[j][d];
                dist += diff * diff;
            }
            dist = qSqrt(dist);
            m_distMatrix[i][j] = dist;
            m_distMatrix[j][i] = dist;
            allDists.append(dist);
        }
    }

    /* 截断距离取所有距离的第p百分位 */
    std::sort(allDists.begin(), allDists.end());
    int idx = qBound(0, static_cast<int>(m_cutoffRatio * allDists.size()),
                     allDists.size() - 1);
    m_dc = allDists[idx];
}

/** @brief 计算局部密度 @param n 点数 @return 密度数组 */
QVector<double> HierarchicalDensity::computeDensity(int n)
{
    QVector<double> density(n, 0.0);
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            if (i == j) continue;
            double d = m_distMatrix[i][j];
            switch (m_method) {
            case DensityMethod::GaussianKernel:
                density[i] += qExp(-(d * d) / (2.0 * m_sigma * m_sigma));
                break;
            case DensityMethod::CutoffRadius:
                if (d < m_dc) density[i] += 1.0;
                break;
            case DensityMethod::KNN:
                if (d > 1e-10) density[i] += 1.0 / d;
                break;
            }
        }
    }
    return density;
}

/** @brief 计算delta值 @param n 点数 @param density 密度 @return delta数组 */
QVector<double> HierarchicalDensity::computeDelta(int n,
                                                   const QVector<double>& density)
{
    QVector<double> delta(n, 0.0);
    for (int i = 0; i < n; ++i) {
        double minDist = std::numeric_limits<double>::max();
        int nearest = -1;
        bool foundHigher = false;
        for (int j = 0; j < n; ++j) {
            if (i == j) continue;
            if (density[j] > density[i] && m_distMatrix[i][j] < minDist) {
                minDist = m_distMatrix[i][j];
                nearest = j;
                foundHigher = true;
            }
        }
        if (foundHigher) {
            delta[i] = minDist;
        } else {
            /* 最高密度点: delta为最大距离 */
            for (int j = 0; j < n; ++j) {
                if (i != j) delta[i] = qMax(delta[i], m_distMatrix[i][j]);
            }
            nearest = -1;
        }
    }
    return delta;
}

/** @brief 选择聚类中心 @param n 点数 @param density 密度 @param delta delta @return 中心索引列表 */
QList<int> HierarchicalDensity::selectCenters(int n,
                                               const QVector<double>& density,
                                               const QVector<double>& delta)
{
    /* 计算 gamma = density * delta，取明显突出的点 */
    QVector<double> gamma(n);
    for (int i = 0; i < n; ++i) {
        gamma[i] = density[i] * delta[i];
    }

    /* 找到gamma中位数作为阈值 */
    QVector<double> sorted = gamma;
    std::sort(sorted.begin(), sorted.end());
    double threshold = sorted[static_cast<int>(sorted.size() * 0.85)];

    QList<int> centers;
    for (int i = 0; i < n; ++i) {
        if (gamma[i] >= threshold) centers.append(i);
    }

    /* 初始化结果 */
    m_results.resize(n);
    for (int i = 0; i < n; ++i) {
        m_results[i].index = i;
        m_results[i].density = density[i];
        m_results[i].delta = delta[i];
        m_results[i].clusterId = -1;
        m_results[i].isCenter = false;
    }

    /* 标记中心 */
    for (int idx : centers) {
        m_results[idx].isCenter = true;
        m_results[idx].clusterId = idx;
    }

    return centers;
}

/** @brief 分配非中心点 @param n 点数 @param centers 中心 @param density 密度 */
void HierarchicalDensity::assignClusters(int n, const QList<int>& centers,
                                          const QVector<double>& density)
{
    if (centers.isEmpty()) return;

    /* 按密度降序排列所有点 */
    QVector<int> order(n);
    for (int i = 0; i < n; ++i) order[i] = i;
    std::sort(order.begin(), order.end(), [&](int a, int b) {
        return density[a] > density[b];
    });

    for (int idx : order) {
        if (m_results[idx].isCenter) continue;
        /* 找到最近且已分配的更高密度点 */
        double minDist = std::numeric_limits<double>::max();
        int nearest = -1;
        for (int j = 0; j < n; ++j) {
            if (j == idx) continue;
            if (density[j] > density[idx] && m_results[j].clusterId >= 0) {
                if (m_distMatrix[idx][j] < minDist) {
                    minDist = m_distMatrix[idx][j];
                    nearest = j;
                }
            }
        }
        if (nearest >= 0) {
            m_results[idx].clusterId = m_results[nearest].clusterId;
            m_results[idx].nearestHigher = nearest;
        }
    }
}

/** @brief Halo噪声点标记 @param n 点数 */
void HierarchicalDensity::assignHalo(int n)
{
    /* 对每个聚类，找到边界密度(最低密度的核心点) */
    QMap<int, double> borderDensity;
    for (int i = 0; i < n; ++i) {
        if (m_results[i].clusterId < 0) continue;
        int cid = m_results[i].clusterId;
        for (int j = 0; j < n; ++j) {
            if (i == j || m_results[j].clusterId < 0) continue;
            if (m_results[j].clusterId != cid) {
                /* 不同聚类的近邻点 */
                if (!borderDensity.contains(cid)
                    || m_results[i].density < borderDensity[cid]) {
                    borderDensity[cid] = m_results[i].density;
                }
            }
        }
    }

    /* 密度低于边界密度的点标记为噪声(clusterId=-1) */
    for (int i = 0; i < n; ++i) {
        int cid = m_results[i].clusterId;
        if (cid >= 0 && borderDensity.contains(cid)) {
            if (m_results[i].density < borderDensity[cid]) {
                m_results[i].clusterId = -1;
            }
        }
    }
}
