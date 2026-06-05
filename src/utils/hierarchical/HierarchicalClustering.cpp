/**
 * @file HierarchicalClustering.cpp
 * @brief 层次聚类实现
 */

#include "HierarchicalClustering.h"
#include <QElapsedTimer>
#include <cmath>
#include <limits>
#include <algorithm>

HierarchicalClustering::HierarchicalClustering(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

QVector<HierarchicalClustering::MergeStep> HierarchicalClustering::fit(
    const QVector<QVector<double>>& data, Linkage linkage)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    QVector<MergeStep> merges;

    if (n < 2) {
        m_stats.totalClustered++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, m_stats.totalClustered);
        return merges;
    }

    /* 初始化: 每个点一个簇 */
    QVector<QVector<int>> clusters(n);
    for (int i = 0; i < n; ++i)
        clusters[i].append(i);

    QVector<bool> active(n, true);

    /* 距离矩阵 */
    int matrixSize = 2 * n;
    QVector<QVector<double>> dist(matrixSize, QVector<double>(matrixSize, -1.0));

    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            double d = euclidean(data[i], data[j]);
            dist[i][j] = d;
            dist[j][i] = d;
        }
    }

    for (int step = 0; step < n - 1; ++step) {
        /* 找最小距离 */
        double minDist = std::numeric_limits<double>::max();
        int bestI = -1, bestJ = -1;

        for (int i = 0; i < matrixSize; ++i) {
            if (!active[i] || clusters[i].isEmpty()) continue;
            for (int j = i + 1; j < matrixSize; ++j) {
                if (!active[j] || clusters[j].isEmpty()) continue;
                if (dist[i][j] < 0) {
                    dist[i][j] = clusterDistance(clusters[i], clusters[j], data, linkage);
                    dist[j][i] = dist[i][j];
                }
                if (dist[i][j] < minDist) {
                    minDist = dist[i][j];
                    bestI = i;
                    bestJ = j;
                }
            }
        }

        if (bestI < 0) break;

        /* 创建新簇 */
        int newIdx = n + step;
        clusters.resize(newIdx + 1);
        clusters[newIdx] = clusters[bestI] + clusters[bestJ];

        MergeStep merge;
        merge.cluster1 = bestI;
        merge.cluster2 = bestJ;
        merge.distance = minDist;
        merge.newSize = clusters[newIdx].size();
        merges.append(merge);

        active[bestI] = false;
        active[bestJ] = false;
        active.append(true);

        /* 预计算新簇到其他活跃簇的距离 */
        for (int k = 0; k < newIdx; ++k) {
            if (!active[k] || clusters[k].isEmpty()) continue;
            dist[newIdx][k] = clusterDistance(clusters[newIdx], clusters[k], data, linkage);
            dist[k][newIdx] = dist[newIdx][k];
        }
    }

    m_stats.totalClustered++;
    m_stats.totalPoints += n;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalClustered;

    emit clusteringCompleted(n, merges.size());
    return merges;
}

QVector<int> HierarchicalClustering::getClusters(const QVector<MergeStep>& merges,
                                                    int nClusters) const
{
    int n = merges.size() + 1;
    QVector<int> labels(n, -1);

    /* 构建合并树 */
    QVector<int> parent(2 * n, -1);
    for (int i = 0; i < merges.size(); ++i) {
        int newIdx = n + i;
        parent[merges[i].cluster1] = newIdx;
        parent[merges[i].cluster2] = newIdx;
    }

    /* 找最后nClusters-1次合并的根 */
    int cutStep = merges.size() - nClusters + 1;
    QVector<int> roots;
    if (cutStep >= 0 && cutStep < merges.size()) {
        roots.append(merges[cutStep].cluster1);
        roots.append(merges[cutStep].cluster2);
    } else {
        roots.append(n + merges.size() - 1);
    }

    /* 添加后续合并中未被合并的节点 */
    for (int i = cutStep + 1; i < merges.size(); ++i)
        roots.append(n + i);

    /* 分配标签 */
    for (int i = 0; i < n; ++i) {
        int node = i;
        while (parent[node] != -1) {
            bool found = false;
            for (int r = 0; r < roots.size(); ++r) {
                if (parent[node] == roots[r] || node == roots[r]) {
                    labels[i] = r;
                    found = true;
                    break;
                }
            }
            if (found) break;
            node = parent[node];
        }
        if (labels[i] < 0) labels[i] = 0;
    }

    return labels;
}

int HierarchicalClustering::optimalClusterCount(const QVector<MergeStep>& merges) const
{
    if (merges.size() < 2) return 1;

    /* 计算不一致系数 */
    QVector<double> diffs;
    for (int i = 1; i < merges.size(); ++i)
        diffs.append(merges[i].distance - merges[i - 1].distance);

    double maxDiff = 0.0;
    int bestK = 1;
    for (int i = 0; i < diffs.size(); ++i) {
        if (diffs[i] > maxDiff) {
            maxDiff = diffs[i];
            bestK = merges.size() - i;
        }
    }
    return qMax(1, bestK);
}

double HierarchicalClustering::euclidean(const QVector<double>& a,
                                            const QVector<double>& b)
{
    double sum = 0.0;
    int dim = qMin(a.size(), b.size());
    for (int i = 0; i < dim; ++i) {
        double d = a[i] - b[i];
        sum += d * d;
    }
    return std::sqrt(sum);
}

double HierarchicalClustering::clusterDistance(const QVector<int>& c1,
                                                  const QVector<int>& c2,
                                                  const QVector<QVector<double>>& data,
                                                  Linkage linkage) const
{
    if (c1.isEmpty() || c2.isEmpty()) return 0.0;

    if (linkage == Single) {
        double minD = std::numeric_limits<double>::max();
        for (int i : c1)
            for (int j : c2)
                minD = qMin(minD, euclidean(data[i], data[j]));
        return minD;
    } else if (linkage == Complete) {
        double maxD = 0.0;
        for (int i : c1)
            for (int j : c2)
                maxD = qMax(maxD, euclidean(data[i], data[j]));
        return maxD;
    } else if (linkage == Average) {
        double sum = 0.0;
        for (int i : c1)
            for (int j : c2)
                sum += euclidean(data[i], data[j]);
        return sum / (c1.size() * c2.size());
    }

    /* Ward */
    int n1 = c1.size(), n2 = c2.size();
    QVector<double> c1Center(data[0].size(), 0.0);
    QVector<double> c2Center(data[0].size(), 0.0);
    for (int i : c1)
        for (int d = 0; d < data[0].size(); ++d)
            c1Center[d] += data[i][d];
    for (int j : c2)
        for (int d = 0; d < data[0].size(); ++d)
            c2Center[d] += data[j][d];
    for (int d = 0; d < data[0].size(); ++d) {
        c1Center[d] /= n1;
        c2Center[d] /= n2;
    }
    double d2 = 0.0;
    for (int d = 0; d < data[0].size(); ++d) {
        double diff = c1Center[d] - c2Center[d];
        d2 += diff * diff;
    }
    return (n1 * n2) * d2 / (n1 + n2);
}

HierarchicalClustering::Stats HierarchicalClustering::stats() const { return m_stats; }

void HierarchicalClustering::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
