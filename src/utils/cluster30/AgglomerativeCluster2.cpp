/**
 * @file AgglomerativeCluster2.cpp
 * @brief 层次聚类增强实现 — Ward/Single/Complete/Average链接/树状图/剪枝
 */

#include "utils/cluster30/AgglomerativeCluster2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/** @brief 构造函数 @param parent 父对象 */
AgglomerativeCluster2::AgglomerativeCluster2(QObject* parent)
    : QObject(parent)
{
}

/** @brief 设置链接方式 @param method 链接方法 */
void AgglomerativeCluster2::setLinkage(Linkage method)
{
    m_linkage = method;
}

/** @brief 设置距离阈值 @param threshold 聚类停止距离 */
void AgglomerativeCluster2::setDistanceThreshold(double threshold)
{
    m_threshold = qMax(0.0, threshold);
}

/**
 * @brief 执行层次聚类
 * @param data 数据点(每行一个样本，每列一个特征)
 * @param k 目标聚类数
 * @return 每个样本的聚类标签
 */
QVector<int> AgglomerativeCluster2::fit(const QVector<QVector<double>>& data, int k)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    if (n == 0) return {};

    k = qBound(1, k, n);
    m_merges.clear();
    m_mergeDistances.clear();

    /* 初始化: 每个点是一个独立簇 */
    QVector<int> labels(n);
    QVector<QVector<int>> clusters(n);
    for (int i = 0; i < n; ++i) {
        labels[i] = i;
        clusters[i].append(i);
    }

    /* 计算初始距离矩阵 */
    QVector<QVector<double>> dist(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            dist[i][j] = dist[j][i] = euclidean(data[i], data[j]);
        }
    }

    /* 记录活跃簇 */
    QVector<bool> alive(n, true);
    int activeCount = n;

    /* 迭代合并最近的两个簇 */
    while (activeCount > k) {
        /* 找最小距离 */
        double minDist = std::numeric_limits<double>::max();
        int mergeI = -1, mergeJ = -1;

        for (int i = 0; i < n; ++i) {
            if (!alive[i]) continue;
            for (int j = i + 1; j < n; ++j) {
                if (!alive[j]) continue;
                if (dist[i][j] < minDist) {
                    minDist = dist[i][j];
                    mergeI = i;
                    mergeJ = j;
                }
            }
        }

        if (mergeI < 0) break;

        /* 检查阈值停止条件 */
        if (m_threshold > 0.0 && minDist > m_threshold) break;

        /* 记录合并 */
        m_merges.append(qMakePair(mergeI, mergeJ));
        m_mergeDistances.append(minDist);

        /* 合并簇: 将mergeJ并入mergeI */
        clusters[mergeI].append(clusters[mergeJ]);
        clusters[mergeJ].clear();
        alive[mergeJ] = false;

        /* 更新距离矩阵 */
        for (int k2 = 0; k2 < n; ++k2) {
            if (!alive[k2] || k2 == mergeI) continue;
            dist[mergeI][k2] = dist[k2][mergeI] =
                clusterDistance(clusters[mergeI], clusters[k2], data);
        }

        --activeCount;
    }

    /* 分配最终标签 */
    int label = 0;
    for (int i = 0; i < n; ++i) {
        if (!alive[i]) continue;
        for (int idx : clusters[i]) {
            labels[idx] = label;
        }
        ++label;
    }

    m_stats.totalClusterings++;
    m_stats.totalPointsProcessed += n;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(qMax(1, m_stats.totalClusterings));

    emit clusteringComplete(label);
    return labels;
}

/**
 * @brief 获取树状图合并记录
 * @return 合并对列表(簇ID对)
 */
QVector<QPair<int, int>> AgglomerativeCluster2::dendrogram() const
{
    return m_merges;
}

/**
 * @brief 在指定层数切割树状图
 * @param k 目标聚类数
 * @return 每个样本的聚类标签
 */
QVector<int> AgglomerativeCluster2::cutTree(int k) const
{
    if (m_merges.isEmpty()) return {};

    int n = 0;
    /* 找最大簇ID推断样本数 */
    for (const auto& merge : m_merges) {
        n = qMax(n, qMax(merge.first, merge.second));
    }
    n += 1;

    k = qBound(1, k, n);

    /* 并查集模拟合并 */
    QVector<int> parent(n);
    for (int i = 0; i < n; ++i) parent[i] = i;

    /* 只执行前 (n - k) 次合并 */
    int mergeCount = n - k;
    for (int i = 0; i < qMin(mergeCount, m_merges.size()); ++i) {
        int a = m_merges[i].first;
        int b = m_merges[i].second;
        /* 将b的根合并到a的根 */
        int rootA = findRoot(parent, a);
        int rootB = findRoot(parent, b);
        if (rootA != rootB) {
            parent[rootB] = rootA;
        }
    }

    /* 压缩路径并分配标签 */
    QVector<int> labels(n, -1);
    int label = 0;
    for (int i = 0; i < n; ++i) {
        int root = findRoot(parent, i);
        if (labels[root] < 0) {
            labels[root] = label++;
        }
        labels[i] = labels[root];
    }

    return labels;
}

/** @brief 重置统计 */
void AgglomerativeCluster2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @brief 计算两个簇之间的距离
 * @param a 簇A的样本索引
 * @param b 簇B的样本索引
 * @param data 原始数据
 * @return 距离值
 */
double AgglomerativeCluster2::clusterDistance(const QVector<int>& a,
    const QVector<int>& b, const QVector<QVector<double>>& data) const
{
    if (a.isEmpty() || b.isEmpty()) return 0.0;

    switch (m_linkage) {
    case Single: {
        /* 单链接: 最小距离 */
        double minD = std::numeric_limits<double>::max();
        for (int ia : a) {
            for (int ib : b) {
                minD = qMin(minD, euclidean(data[ia], data[ib]));
            }
        }
        return minD;
    }
    case Complete: {
        /* 全链接: 最大距离 */
        double maxD = 0.0;
        for (int ia : a) {
            for (int ib : b) {
                maxD = qMax(maxD, euclidean(data[ia], data[ib]));
            }
        }
        return maxD;
    }
    case Average: {
        /* 平均链接: 所有点对的平均距离 */
        double sum = 0.0;
        int count = 0;
        for (int ia : a) {
            for (int ib : b) {
                sum += euclidean(data[ia], data[ib]);
                ++count;
            }
        }
        return (count > 0) ? sum / count : 0.0;
    }
    case Ward: {
        /* Ward链接: 基于簇内方差增量 */
        int na = a.size();
        int nb = b.size();

        QVector<double> centroidA(data[0].size(), 0.0);
        QVector<double> centroidB(data[0].size(), 0.0);

        for (int ia : a) {
            for (int d = 0; d < data[ia].size(); ++d) {
                centroidA[d] += data[ia][d];
            }
        }
        for (int d = 0; d < centroidA.size(); ++d) {
            centroidA[d] /= na;
        }

        for (int ib : b) {
            for (int d = 0; d < data[ib].size(); ++d) {
                centroidB[d] += data[ib][d];
            }
        }
        for (int d = 0; d < centroidB.size(); ++d) {
            centroidB[d] /= nb;
        }

        double dist = euclidean(centroidA, centroidB);
        return (static_cast<double>(na * nb) / (na + nb)) * dist * dist;
    }
    }
    return 0.0;
}

/**
 * @brief 计算两个向量的欧氏距离
 * @param a 向量A
 * @param b 向量B
 * @return 欧氏距离
 */
double AgglomerativeCluster2::euclidean(const QVector<double>& a,
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

/**
 * @brief 并查集查找根节点(带路径压缩)
 * @param parent 父指针数组
 * @param x 当前节点
 * @return 根节点
 */
int AgglomerativeCluster2::findRoot(QVector<int>& parent, int x) const
{
    while (parent[x] != x) {
        parent[x] = parent[parent[x]];
        x = parent[x];
    }
    return x;
}
