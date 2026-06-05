/**
 * @file HierarchicalCluster7.cpp
 * @brief 层次聚类算法实现 — 支持 single/complete/average/ward 链接准则
 */

#include "utils/cluster74/HierarchicalCluster7.h"

#include <QElapsedTimer>
#include <QMap>
#include <QtMath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
HierarchicalCluster7::HierarchicalCluster7(QObject* parent)
    : QObject(parent)
{
}

/** @brief 设置链接准则 @param linkage 链接类型: single/complete/average/ward */
void HierarchicalCluster7::setLinkage(const QString& linkage)
{
    m_linkage = linkage;
}

/** @brief 重置统计数据 */
void HierarchicalCluster7::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 计算两个簇之间的距离
 * @param dists 全局距离矩阵
 * @param members 每个簇的成员索引集合
 * @param i 簇i索引
 * @param j 簇j索引
 * @return 簇间距离
 */
static double clusterDistance(const QVector<QVector<double>>& dists,
                              const QVector<QVector<int>>& members,
                              int i, int j, const QString& linkage)
{
    double result = 0.0;
    if (linkage == "single") {
        result = 1e18;
        for (int ai : members[i]) {
            for (int bi : members[j]) {
                result = std::min(result, dists[ai][bi]);
            }
        }
    } else if (linkage == "complete") {
        result = 0.0;
        for (int ai : members[i]) {
            for (int bi : members[j]) {
                result = std::max(result, dists[ai][bi]);
            }
        }
    } else if (linkage == "average") {
        double sum = 0.0;
        int count = 0;
        for (int ai : members[i]) {
            for (int bi : members[j]) {
                sum += dists[ai][bi];
                ++count;
            }
        }
        result = (count > 0) ? sum / count : 0.0;
    } else {
        /* Ward: 基于增量方差 */
        double sum = 0.0;
        int count = 0;
        for (int ai : members[i]) {
            for (int bi : members[j]) {
                sum += dists[ai][bi] * dists[ai][bi];
                ++count;
            }
        }
        int ni = members[i].size();
        int nj = members[j].size();
        result = (count > 0) ? (2.0 * sum / count) * ni * nj / (ni + nj) : 0.0;
    }
    return result;
}

/**
 * @brief 执行层次聚类
 * @param data 输入数据矩阵(样本×特征)
 * @param targetClusters 目标簇数量
 * @return 合并历史，每项为(被合并簇i, 被合并簇j)
 *
 * 使用凝聚策略: 每步合并距离最近的两个簇，直到达到目标簇数。
 */
QVector<QPair<int,int>> HierarchicalCluster7::fit(
    const QVector<QVector<double>>& data, int targetClusters)
{
    QElapsedTimer timer;
    timer.start();

    m_mergeHistory.clear();
    m_mergeHeights.clear();

    int n = data.size();
    if (n < 2 || targetClusters < 1) {
        return m_mergeHistory;
    }
    targetClusters = std::min(targetClusters, n);

    /* 计算欧氏距离矩阵 */
    int dim = data[0].size();
    QVector<QVector<double>> dists(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            double sum = 0.0;
            for (int d = 0; d < dim; ++d) {
                double diff = data[i][d] - data[j][d];
                sum += diff * diff;
            }
            dists[i][j] = dists[j][i] = qSqrt(sum);
        }
    }

    /* 初始化: 每个点自成一个簇 */
    QVector<QVector<int>> members(n);
    QVector<bool> alive(n, true);
    for (int i = 0; i < n; ++i) {
        members[i] = {i};
    }

    int numMerges = n - targetClusters;
    for (int step = 0; step < numMerges; ++step) {
        /* 找距离最近的两个活簇 */
        double minDist = 1e18;
        int bestI = -1, bestJ = -1;
        for (int i = 0; i < n; ++i) {
            if (!alive[i]) continue;
            for (int j = i + 1; j < n; ++j) {
                if (!alive[j]) continue;
                double d = clusterDistance(dists, members, i, j, m_linkage);
                if (d < minDist) {
                    minDist = d;
                    bestI = i;
                    bestJ = j;
                }
            }
        }
        if (bestI < 0) break;

        /* 合并 j → i */
        m_mergeHistory.append({bestI, bestJ});
        m_mergeHeights.append(minDist);
        members[bestI].append(members[bestJ]);
        members[bestJ].clear();
        alive[bestJ] = false;
    }

    m_stats.totalMerges += m_mergeHistory.size();
    m_stats.totalClusters += targetClusters;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / std::max(1, m_stats.totalMerges);

    emit clusteringCompleted(targetClusters,
        m_mergeHeights.isEmpty() ? 0.0 : m_mergeHeights.last());
    return m_mergeHistory;
}

/**
 * @brief 获取指定截断的簇标签
 * @param numClusters 目标簇数
 * @return 每个样本的簇标签
 *
 * 根据合并历史回溯，前 numClusters 个活簇分配不同标签。
 */
QVector<int> HierarchicalCluster7::getLabels(int numClusters) const
{
    int n = m_mergeHistory.size() + numClusters;
    QVector<int> labels(n, -1);
    QVector<int> clusterId(n);
    for (int i = 0; i < n; ++i) clusterId[i] = i;

    /* 执行前 n-k 次合并 */
    int merges = m_mergeHistory.size();
    for (int s = 0; s < merges; ++s) {
        int ci = m_mergeHistory[s].first;
        int cj = m_mergeHistory[s].second;
        for (int p = 0; p < n; ++p) {
            if (clusterId[p] == cj) clusterId[p] = ci;
        }
    }

    /* 给每个活簇分配标签 */
    QMap<int, int> remap;
    int label = 0;
    for (int i = 0; i < n; ++i) {
        if (!remap.contains(clusterId[i])) {
            remap[clusterId[i]] = label++;
        }
        labels[i] = remap[clusterId[i]];
    }
    return labels;
}

/** @brief 获取合并高度序列 */
QVector<double> HierarchicalCluster7::mergeHeights() const
{
    return m_mergeHeights;
}
