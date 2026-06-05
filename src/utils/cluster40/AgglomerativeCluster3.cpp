/**
 * @file AgglomerativeCluster3.cpp
 * @brief 层次聚类3实现 — Ward方差最小+树状图剪枝
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/cluster40/AgglomerativeCluster3.h"

#include <QElapsedTimer>
#include <QMap>
#include <QtMath>

#include <algorithm>
#include <limits>
#include <cmath>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父对象
 */
AgglomerativeCluster3::AgglomerativeCluster3(QObject* parent)
    : QObject(parent)
{
    setObjectName(QStringLiteral("AgglomerativeCluster3"));
}

/**
 * @brief 设置链接策略
 *
 * 支持 "ward"（Ward方差最小化）、"single"（单链接）、
 * "complete"（全链接）三种策略。
 *
 * @param linkage 链接策略名称
 */
void AgglomerativeCluster3::setLinkage(const QString& linkage)
{
    if (linkage == "ward" || linkage == "single" || linkage == "complete") {
        m_linkage = linkage;
    }
}

/**
 * @brief 设置目标簇数
 * @param k 簇数（最小为2）
 */
void AgglomerativeCluster3::setNumClusters(int k)
{
    m_numClusters = qMax(2, k);
}

/**
 * @brief 执行层次聚类
 *
 * 从每个数据点作为一个簇开始，每次合并距离最近的两个簇，
 * 直到达到目标簇数。记录完整合并历史用于树状图。
 *
 * @param data 数据点集合
 * @return 每个数据点的簇标签
 */
QVector<int> AgglomerativeCluster3::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    if (n == 0) return {};

    m_stats.totalPointsProcessed += n;

    /* 初始化：每个点为一个簇 */
    QVector<QVector<int>> clusters(n);
    for (int i = 0; i < n; ++i) clusters[i].append(i);

    /* 距离矩阵 */
    QVector<double> dist(n * n, 0.0);
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            double d = 0.0;
            int dim = qMin(data[i].size(), data[j].size());
            for (int dd = 0; dd < dim; ++dd) {
                double diff = data[i][dd] - data[j][dd];
                d += diff * diff;
            }
            dist[i * n + j] = qSqrt(d);
            dist[j * n + i] = dist[i * n + j];
        }
    }

    /* 活跃簇标记 */
    QVector<bool> active(n, true);
    m_merges.clear();
    m_mergeDist.clear();

    int numActive = n;
    int nextLabel = n;

    while (numActive > m_numClusters) {
        /* 寻找最近的两个簇 */
        double minDist = std::numeric_limits<double>::max();
        int bestI = -1, bestJ = -1;

        for (int i = 0; i < n; ++i) {
            if (!active[i]) continue;
            for (int j = i + 1; j < n; ++j) {
                if (!active[j]) continue;
                double d = dist[i * n + j];
                if (d < minDist) {
                    minDist = d;
                    bestI = i;
                    bestJ = j;
                }
            }
        }

        if (bestI < 0 || bestJ < 0) break;

        /* 记录合并历史 */
        m_merges.append({bestI, bestJ});
        m_mergeDist.append(minDist);
        m_stats.totalMerges++;

        /* 合并簇 */
        clusters[bestI].append(clusters[bestJ]);
        active[bestJ] = false;

        /* 更新距离矩阵（Lance-Williams公式简化） */
        for (int k = 0; k < n; ++k) {
            if (!active[k] || k == bestI) continue;

            double newDist = 0.0;
            if (m_linkage == "single") {
                newDist = qMin(dist[bestI * n + k], dist[bestJ * n + k]);
            } else if (m_linkage == "complete") {
                newDist = qMax(dist[bestI * n + k], dist[bestJ * n + k]);
            } else {
                /* Ward方法：使用簇大小加权 */
                int ni = clusters[bestI].size();
                int nj = clusters[bestJ].size();
                int nk = clusters[k].size();
                double dik = dist[bestI * n + k];
                double djk = dist[bestJ * n + k];
                double dij = minDist;
                newDist = qSqrt(
                    ((ni + nk) * dik * dik +
                     (nj + nk) * djk * djk -
                     nk * dij * dij) / (ni + nj + nk)
                );
            }

            dist[bestI * n + k] = newDist;
            dist[k * n + bestI] = newDist;
        }

        numActive--;
        nextLabel++;
    }

    /* 收集簇标签 */
    QVector<int> labels(n, -1);
    int label = 0;
    for (int i = 0; i < n; ++i) {
        if (!active[i]) continue;
        for (int idx : clusters[i]) {
            labels[idx] = label;
        }
        label++;
    }

    m_stats.totalClusterings++;

    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalClusterings;

    emit clusteringCompleted(n, label);
    return labels;
}

/**
 * @brief 在指定高度剪裁树状图
 *
 * 返回k个簇的成员列表。基于合并距离序列，
 * 在第(n-k)次合并处截断。
 *
 * @param k 目标簇数
 * @return 各簇的成员索引列表
 */
QVector<QVector<int>> AgglomerativeCluster3::cutTree(int k) const
{
    int n = m_merges.size() + k;
    if (k <= 0 || k > n) return {};

    /* 并查集跟踪合并 */
    QVector<int> parent(n * 2, -1);
    for (int i = 0; i < n; ++i) parent[i] = i;

    auto find = [&](int x) -> int {
        while (parent[x] != x) { parent[x] = parent[parent[x]]; x = parent[x]; }
        return x;
    };

    /* 执行前(n-k)次合并 */
    int numMerges = n - k;
    for (int i = 0; i < numMerges && i < m_merges.size(); ++i) {
        int a = find(m_merges[i].first);
        int b = find(m_merges[i].second);
        int newId = n + i;
        parent[newId] = newId;
        parent[a] = newId;
        parent[b] = newId;
    }

    /* 收集结果 */
    QMap<int, QVector<int>> groups;
    for (int i = 0; i < n; ++i) {
        groups[find(i)].append(i);
    }

    QVector<QVector<int>> result;
    for (auto it = groups.begin(); it != groups.end(); ++it) {
        result.append(it.value());
    }
    return result;
}

/**
 * @brief Ward方差距离计算
 *
 * Ward方法使合并后方差增量最小化。
 * 距离 = sqrt(2 * n_a * n_b / (n_a + n_b) * ||centroid_a - centroid_b||^2)
 *
 * @param data 原始数据
 * @param c1 第一个簇的索引集合
 * @param c2 第二个簇的索引集合
 * @return Ward距离
 */
double AgglomerativeCluster3::wardDistance(
    const QVector<QVector<double>>& data,
    const QVector<int>& c1, const QVector<int>& c2) const
{
    if (c1.isEmpty() || c2.isEmpty()) return 0.0;
    int dim = data[0].size();

    /* 计算质心 */
    QVector<double> cent1(dim, 0.0), cent2(dim, 0.0);
    for (int idx : c1) {
        for (int d = 0; d < dim; ++d) cent1[d] += data[idx][d];
    }
    for (int d = 0; d < dim; ++d) cent1[d] /= c1.size();

    for (int idx : c2) {
        for (int d = 0; d < dim; ++d) cent2[d] += data[idx][d];
    }
    for (int d = 0; d < dim; ++d) cent2[d] /= c2.size();

    /* Ward距离 */
    double distSq = 0.0;
    for (int d = 0; d < dim; ++d) {
        double diff = cent1[d] - cent2[d];
        distSq += diff * diff;
    }

    double n1 = c1.size(), n2 = c2.size();
    return qSqrt(2.0 * n1 * n2 / (n1 + n2) * distSq);
}

/**
 * @brief 单链接距离（最小距离）
 */
double AgglomerativeCluster3::singleLink(
    const QVector<QVector<double>>& data,
    const QVector<int>& c1, const QVector<int>& c2) const
{
    double minD = std::numeric_limits<double>::max();
    for (int i : c1) {
        for (int j : c2) {
            double d = 0.0;
            int dim = qMin(data[i].size(), data[j].size());
            for (int dd = 0; dd < dim; ++dd) {
                double diff = data[i][dd] - data[j][dd];
                d += diff * diff;
            }
            minD = qMin(minD, qSqrt(d));
        }
    }
    return minD;
}

/**
 * @brief 全链接距离（最大距离）
 */
double AgglomerativeCluster3::completeLink(
    const QVector<QVector<double>>& data,
    const QVector<int>& c1, const QVector<int>& c2) const
{
    double maxD = 0.0;
    for (int i : c1) {
        for (int j : c2) {
            double d = 0.0;
            int dim = qMin(data[i].size(), data[j].size());
            for (int dd = 0; dd < dim; ++dd) {
                double diff = data[i][dd] - data[j][dd];
                d += diff * diff;
            }
            maxD = qMax(maxD, qSqrt(d));
        }
    }
    return maxD;
}

/**
 * @brief 重置所有统计数据
 */
void AgglomerativeCluster3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
