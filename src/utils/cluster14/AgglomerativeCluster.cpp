/**
 * @file AgglomerativeCluster.cpp
 * @brief 层次聚合聚类实现
 */

#include "AgglomerativeCluster.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>
#include <limits>

AgglomerativeCluster::AgglomerativeCluster(Linkage linkage, QObject* parent)
    : QObject(parent)
    , m_linkage(linkage)
    , m_timeSum(0.0)
{
}

void AgglomerativeCluster::fit(const QVector<double>& data, int dims)
{
    QElapsedTimer timer;
    timer.start();

    m_dims = dims;
    m_n = data.size() / dims;
    m_nodes.clear();
    m_merges.clear();

    /* 初始化: 每个点为一个簇 */
    m_nodes.reserve(2 * m_n);
    for (int i = 0; i < m_n; ++i) {
        ClusterNode node;
        node.id = i;
        node.size = 1;
        node.left = -1;
        node.right = -1;
        node.height = 0.0;
        node.centroid.resize(dims);
        for (int d = 0; d < dims; ++d)
            node.centroid[d] = data[i * dims + d];
        node.members.append(i);
        m_nodes.append(node);
    }

    /* 距离矩阵(上三角) */
    int totalClusters = m_n;
    QVector<QVector<double>> dist(m_n, QVector<double>(m_n, 0.0));
    for (int i = 0; i < m_n; ++i) {
        for (int j = i + 1; j < m_n; ++j) {
            dist[i][j] = clusterDistance(m_nodes[i], m_nodes[j]);
            dist[j][i] = dist[i][j];
        }
    }

    /* 活跃簇集合 */
    QVector<bool> active(m_n, true);

    /* 迭代合并 */
    for (int step = 0; step < m_n - 1; ++step) {
        /* 找最小距离对 */
        double minDist = std::numeric_limits<double>::max();
        int bestI = -1, bestJ = -1;
        for (int i = 0; i < m_nodes.size(); ++i) {
            if (!active[i]) continue;
            for (int j = i + 1; j < m_nodes.size(); ++j) {
                if (!active[j]) continue;
                if (dist[i][j] < minDist) {
                    minDist = dist[i][j];
                    bestI = i;
                    bestJ = j;
                }
            }
        }

        if (bestI < 0) break;

        /* 创建合并节点 */
        ClusterNode merged;
        merged.id = m_nodes.size();
        merged.size = m_nodes[bestI].size + m_nodes[bestJ].size;
        merged.left = bestI;
        merged.right = bestJ;
        merged.height = minDist;
        merged.centroid.resize(m_dims);
        for (int d = 0; d < m_dims; ++d) {
            merged.centroid[d] =
                (m_nodes[bestI].size * m_nodes[bestI].centroid[d] +
                 m_nodes[bestJ].size * m_nodes[bestJ].centroid[d]) /
                merged.size;
        }
        merged.members = m_nodes[bestI].members + m_nodes[bestJ].members;

        /* 记录合并 */
        MergeRecord rec;
        rec.clusterA = bestI;
        rec.clusterB = bestJ;
        rec.distance = minDist;
        rec.newSize = merged.size;
        m_merges.append(rec);

        /* 更新距离矩阵 */
        dist.append(QVector<double>(m_nodes.size() + 1, 0.0));
        for (QVector<double>& row : dist)
            row.append(0.0);
        int newIdx = m_nodes.size();
        for (int k = 0; k < m_nodes.size(); ++k) {
            if (!active[k] || k == bestI || k == bestJ) continue;
            double d = clusterDistance(merged, m_nodes[k]);
            dist[newIdx][k] = d;
            dist[k][newIdx] = d;
        }

        /* 标记旧簇为不活跃 */
        active[bestI] = false;
        active[bestJ] = false;
        active.append(true);

        m_nodes.append(merged);
        emit mergeCompleted(bestI, bestJ, minDist);
    }

    /* 计算惯性 */
    m_inertia = 0.0;
    if (m_nodes.isEmpty()) {
        m_stats.totalFits++;
        m_stats.totalPointsProcessed += m_n;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFits;
        return;
    }

    /* 找根节点(最后合并的) */
    int root = -1;
    for (int i = m_nodes.size() - 1; i >= 0; --i) {
        if (active[i]) { root = i; break; }
    }
    if (root >= 0) {
        for (int idx : m_nodes[root].members) {
            for (int d = 0; d < m_dims; ++d) {
                double diff = data[idx * dims + d] - m_nodes[root].centroid[d];
                m_inertia += diff * diff;
            }
        }
    }

    m_stats.totalFits++;
    m_stats.totalMerges += m_n - 1;
    m_stats.totalPointsProcessed += m_n;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFits;
}

QVector<int> AgglomerativeCluster::labels(int k) const
{
    QVector<int> result(m_n, -1);
    if (m_nodes.isEmpty() || k <= 0) return result;

    /* 找到k个簇: 截断合并历史 */
    int cutMerges = m_nodes.size() - m_n - (k - 1);
    if (cutMerges < 0) cutMerges = 0;

    /* 标记哪些节点是活跃的 */
    QVector<bool> isCluster(m_nodes.size(), false);
    /* 从根开始，不合并最后(k-1)个 */
    if (k >= m_n) {
        for (int i = 0; i < m_n; ++i) result[i] = i;
        return result;
    }

    /* BFS从根向下，在合适位置停止 */
    int rootIdx = m_nodes.size() - 1;
    QSet<int> clusterRoots;
    clusterRoots.insert(rootIdx);

    while (clusterRoots.size() < k) {
        /* 找最高(最大height)的内部节点 */
        int bestNode = -1;
        double maxHeight = -1;
        for (int idx : clusterRoots) {
            if (m_nodes[idx].left < 0) continue;
            if (m_nodes[idx].height > maxHeight) {
                maxHeight = m_nodes[idx].height;
                bestNode = idx;
            }
        }
        if (bestNode < 0) break;

        clusterRoots.remove(bestNode);
        clusterRoots.insert(m_nodes[bestNode].left);
        clusterRoots.insert(m_nodes[bestNode].right);
    }

    int label = 0;
    for (int idx : clusterRoots) {
        extractLabels(idx, label, result);
        label++;
    }

    return result;
}

int AgglomerativeCluster::clusterCountAtDistance(double distance) const
{
    int count = 0;
    for (int i = 0; i < m_nodes.size(); ++i) {
        bool isRoot = true;
        for (const auto& merge : m_merges) {
            if (merge.clusterA == i || merge.clusterB == i) {
                isRoot = false;
                break;
            }
        }
        if (isRoot) {
            if (m_nodes[i].left >= 0 && m_nodes[i].height <= distance)
                continue;
            count++;
        }
    }
    return count;
}

void AgglomerativeCluster::extractLabels(int nodeId, int label,
                                          QVector<int>& out) const
{
    if (nodeId < 0 || nodeId >= m_nodes.size()) return;
    const ClusterNode& node = m_nodes[nodeId];
    if (node.left < 0) {
        for (int idx : node.members)
            out[idx] = label;
    } else {
        extractLabels(node.left, label, out);
        extractLabels(node.right, label, out);
    }
}

double AgglomerativeCluster::clusterDistance(const ClusterNode& a,
                                              const ClusterNode& b) const
{
    switch (m_linkage) {
    case Single: {
        /* 单链接: 两簇间最小距离(用质心近似) */
        return euclidean(a.centroid, b.centroid) * 0.7;
    }
    case Complete: {
        /* 全链接: 两簇间最大距离(用质心近似) */
        return euclidean(a.centroid, b.centroid) * 1.3;
    }
    case Average: {
        /* 平均链接: 所有pair平均(质心距离) */
        return euclidean(a.centroid, b.centroid);
    }
    case Ward: {
        /* Ward: 合并后方差增量 */
        double dist = euclidean(a.centroid, b.centroid);
        int totalSize = a.size + b.size;
        return dist * dist * (a.size * b.size) / totalSize;
    }
    }
    return 0.0;
}

double AgglomerativeCluster::euclidean(const QVector<double>& p1,
                                        const QVector<double>& p2) const
{
    double sum = 0.0;
    int d = qMin(p1.size(), p2.size());
    for (int i = 0; i < d; ++i) {
        double diff = p1[i] - p2[i];
        sum += diff * diff;
    }
    return std::sqrt(sum);
}

void AgglomerativeCluster::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
