/**
 * @file DBSCAN4.cpp
 * @brief 密度聚类增强实现 — 自适应epsilon/HDBSCAN*层次/稳定簇提取
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/cluster27/DBSCAN4.h"

#include <QElapsedTimer>
#include <QtMath>

#include <algorithm>
#include <vector>

/** @brief 构造函数 @param parent 父对象 */
DBSCAN4::DBSCAN4(QObject* parent)
    : QObject(parent)
{
}

/** @brief 设置最小邻域点数 @param minPts 核心点邻域最小样本数 */
void DBSCAN4::setMinPoints(int minPts)
{
    m_minPts = qMax(2, minPts);
}

/** @brief 启用或关闭自适应epsilon估计 @param enable true则自动估计半径 */
void DBSCAN4::setAutoEpsilon(bool enable)
{
    m_autoEpsilon = enable;
}

/** @brief 计算两个样本之间的欧氏距离 @param a 向量a @param b 向量b @return 欧氏距离 */
double DBSCAN4::euclideanDist(const QVector<double>& a,
                               const QVector<double>& b) const
{
    double sum = 0.0;
    int dim = qMin(a.size(), b.size());
    for (int i = 0; i < dim; ++i) {
        double d = a[i] - b[i];
        sum += d * d;
    }
    return qSqrt(sum);
}

/** @brief 基于k-distance曲线的knee点估计最优epsilon
 *  @param data 输入数据集
 *  @return 估计的epsilon半径
 */
double DBSCAN4::estimateEpsilon(const QVector<QVector<double>>& data) const
{
    int n = data.size();
    if (n < m_minPts + 1) {
        return 1.0;
    }

    /* 计算每个点到其第k近邻的距离 */
    int k = m_minPts;
    std::vector<double> kDist(n);
    for (int i = 0; i < n; ++i) {
        std::vector<double> dists;
        dists.reserve(n - 1);
        for (int j = 0; j < n; ++j) {
            if (i != j) {
                dists.push_back(euclideanDist(data[i], data[j]));
            }
        }
        std::sort(dists.begin(), dists.end());
        kDist[i] = dists[qMin(k - 1, static_cast<int>(dists.size()) - 1)];
    }

    /* 排序后寻找knee点(最大曲率位置) */
    std::sort(kDist.begin(), kDist.end());
    double maxCurv = 0.0;
    double bestEps = kDist.back();
    for (int i = 1; i < n - 1; ++i) {
        double dx = kDist[i + 1] - kDist[i - 1];
        double dy = kDist[i + 1] + kDist[i - 1] - 2.0 * kDist[i];
        if (dx > 1e-12) {
            double curv = qAbs(dy) / (dx * dx + 1e-12);
            if (curv > maxCurv) {
                maxCurv = curv;
                bestEps = kDist[i];
            }
        }
    }

    return qMax(bestEps, 1e-6);
}

/** @brief 查询给定点半径epsilon内的所有邻居索引
 *  @param data 数据集
 *  @param idx 查询点索引
 *  @param eps 半径
 *  @return 邻居索引列表
 */
QVector<int> DBSCAN4::regionQuery(const QVector<QVector<double>>& data,
                                   int idx, double eps) const
{
    QVector<int> neighbors;
    double eps2 = eps * eps;
    for (int i = 0; i < data.size(); ++i) {
        if (i == idx) continue;
        double sum = 0.0;
        int dim = qMin(data[idx].size(), data[i].size());
        bool within = true;
        for (int d = 0; d < dim; ++d) {
            double dd = data[idx][d] - data[i][d];
            sum += dd * dd;
            if (sum > eps2) {
                within = false;
                break;
            }
        }
        if (within && sum <= eps2) {
            neighbors.append(i);
        }
    }
    return neighbors;
}

/** @brief 执行DBSCAN聚类，返回每个点的标签(-1为噪声) @param data 输入数据集 @return 标签向量 */
QVector<int> DBSCAN4::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    m_labels.fill(-1, n);

    if (n == 0) {
        emit clusteringComplete(0, 0);
        return m_labels;
    }

    /* 确定epsilon */
    double eps = m_epsilon;
    if (m_autoEpsilon || eps <= 0.0) {
        eps = estimateEpsilon(data);
        m_epsilon = eps;
    }

    int clusterId = 0;
    QVector<bool> visited(n, false);

    for (int i = 0; i < n; ++i) {
        if (visited[i]) continue;
        visited[i] = true;

        QVector<int> neighbors = regionQuery(data, i, eps);
        if (neighbors.size() < m_minPts) {
            /* 暂标记为噪声，后续可能被其他簇吸收 */
            continue;
        }

        /* 开始扩展新簇 */
        m_labels[i] = clusterId;
        QVector<int> seeds = neighbors;
        int idx = 0;
        while (idx < seeds.size()) {
            int cur = seeds[idx];
            if (!visited[cur]) {
                visited[cur] = true;
                QVector<int> curNeighbors = regionQuery(data, cur, eps);
                if (curNeighbors.size() >= m_minPts) {
                    for (int nb : curNeighbors) {
                        if (!seeds.contains(nb)) {
                            seeds.append(nb);
                        }
                    }
                }
            }
            if (m_labels[cur] < 0) {
                m_labels[cur] = clusterId;
            }
            ++idx;
        }
        ++clusterId;
    }

    /* 统计 */
    m_stats.totalClusterings++;
    m_stats.totalPointsProcessed += n;
    qint64 elapsed = timer.elapsed();
    m_timeSum += static_cast<double>(elapsed);
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalClusterings;

    int noiseCount = 0;
    for (int lb : m_labels) {
        if (lb < 0) ++noiseCount;
    }
    emit clusteringComplete(clusterId, noiseCount);
    return m_labels;
}

/** @brief 计算所有点的核心距离(到第minPts近邻的距离)
 *  @param data 输入数据集
 *  @return (核心距离, 邻居数量) 对列表
 */
QVector<QPair<double,int>> DBSCAN4::computeCoreDistances(
    const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    QVector<QPair<double,int>> result(n);

    for (int i = 0; i < n; ++i) {
        std::vector<double> dists;
        dists.reserve(n - 1);
        for (int j = 0; j < n; ++j) {
            if (i != j) {
                dists.push_back(euclideanDist(data[i], data[j]));
            }
        }
        std::sort(dists.begin(), dists.end());
        int k = qMin(m_minPts, static_cast<int>(dists.size()));
        if (k > 0) {
            result[i] = qMakePair(dists[k - 1], k);
        } else {
            result[i] = qMakePair(0.0, 0);
        }
    }

    m_stats.totalPointsProcessed += n;
    qint64 elapsed = timer.elapsed();
    m_timeSum += static_cast<double>(elapsed);
    if (m_stats.totalClusterings > 0) {
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalClusterings;
    }
    return result;
}

/** @brief 从HDBSCAN*凝聚树中提取稳定簇
 *  @param condensedTree 凝聚树(距离,簇大小)对列表
 *  @param minClusterSize 最小簇大小阈值
 *  @return 每个簇的代表性标签
 */
QVector<int> DBSCAN4::extractStableClusters(
    const QVector<QPair<double,int>>& condensedTree,
    double minClusterSize)
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> stableLabels;
    if (condensedTree.isEmpty()) {
        return stableLabels;
    }

    /* 按距离降序排列 — 模拟层次分裂过程 */
    QVector<QPair<double,int>> sorted = condensedTree;
    std::sort(sorted.begin(), sorted.end(),
              [](const QPair<double,int>& a, const QPair<double,int>& b) {
                  return a.first > b.first;
              });

    int label = 0;
    for (const auto& entry : sorted) {
        if (entry.second >= static_cast<int>(minClusterSize)) {
            /* 检查是否为稳定簇: 子簇规模显著大于父簇的分裂 */
            bool isStable = true;
            if (stableLabels.size() > 0 && entry.second < minClusterSize * 1.5) {
                isStable = false;
            }
            if (isStable) {
                stableLabels.append(label++);
            }
        }
    }

    m_stats.totalClusterings++;
    qint64 elapsed = timer.elapsed();
    m_timeSum += static_cast<double>(elapsed);
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalClusterings;
    return stableLabels;
}

/** @brief 返回最近一次fit的簇数量(不含噪声) @return 簇数量 */
int DBSCAN4::clusterCount() const
{
    if (m_labels.isEmpty()) return 0;
    int maxLabel = -1;
    for (int lb : m_labels) {
        if (lb > maxLabel) maxLabel = lb;
    }
    return maxLabel + 1;
}

/** @brief 返回每个簇的大小 @return 各簇样本数列表 */
QVector<double> DBSCAN4::clusterSizes() const
{
    int nc = clusterCount();
    if (nc == 0) return QVector<double>();

    QVector<double> sizes(nc, 0.0);
    for (int lb : m_labels) {
        if (lb >= 0 && lb < nc) {
            sizes[lb] += 1.0;
        }
    }
    return sizes;
}

/** @brief 重置所有统计计数器 */
void DBSCAN4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
