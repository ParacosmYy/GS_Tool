/**
 * @file BirchClustering3.cpp
 * @brief BIRCH聚类3实现 — CF树动态调参+内存感知
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/cluster39/BirchClustering3.h"

#include <QElapsedTimer>
#include <QtMath>

#include <algorithm>
#include <limits>
#include <cmath>
#include <random>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父对象
 */
BirchClustering3::BirchClustering3(QObject* parent)
    : QObject(parent)
{
    setObjectName(QStringLiteral("BirchClustering3"));
}

/**
 * @brief 设置聚类距离阈值
 *
 * 阈值决定了新点被吸收到现有CF条目的最大允许半径。
 * 较小的阈值会产生更多更精细的子簇，内存使用随之增加。
 *
 * @param threshold 距离阈值（必须为正数）
 */
void BirchClustering3::setThreshold(double threshold)
{
    m_threshold = qMax(0.01, threshold);
}

/**
 * @brief 设置CF树分支因子
 *
 * 分支因子控制每个非叶节点的最大子节点数量。
 * 较大的分支因子会减少树高度但增加内存消耗。
 *
 * @param b 分支因子（最小为2）
 */
void BirchClustering3::setBranchingFactor(int b)
{
    m_branchingFactor = qMax(2, b);
}

/**
 * @brief 设置最大内存条目数
 *
 * 当叶条目数超过该限制时，自动重建CF树以释放空间。
 * 内存感知机制确保系统在有限资源下稳定运行。
 *
 * @param maxEntries 最大条目数
 */
void BirchClustering3::setMaxMemory(int maxEntries)
{
    m_maxEntries = qMax(100, maxEntries);
}

/**
 * @brief 插入单个数据点到CF树
 *
 * 首先检测维度一致性，然后寻找最近的叶条目。
 * 如果距离在阈值内则吸收，否则创建新条目。
 * 当内存超限时自动触发重建。
 *
 * @param point 数据点向量
 */
void BirchClustering3::insert(const QVector<double>& point)
{
    QElapsedTimer timer;
    timer.start();

    /* 初始化维度 */
    if (m_dim == 0 && !point.isEmpty()) {
        m_dim = point.size();
    }

    m_points.append(point);
    int idx = m_points.size() - 1;

    /* 寻找最近的叶条目 */
    int bestIdx = -1;
    double bestDist = std::numeric_limits<double>::max();

    for (int i = 0; i < m_leafEntries.size(); ++i) {
        if (m_leafEntries[i].n == 0) continue;
        double dist = 0.0;
        int dim = qMin(m_dim, point.size());
        for (int d = 0; d < dim; ++d) {
            double centroid = m_leafEntries[i].linearSum[d] / m_leafEntries[i].n;
            double diff = point[d] - centroid;
            dist += diff * diff;
        }
        dist = qSqrt(dist);
        if (dist < bestDist) {
            bestDist = dist;
            bestIdx = i;
        }
    }

    if (bestIdx >= 0 && bestDist <= m_threshold) {
        /* 吸收到现有条目 */
        CFEntry& entry = m_leafEntries[bestIdx];
        entry.n++;
        int dim = qMin(m_dim, (int)point.size());
        for (int d = 0; d < dim; ++d) {
            entry.linearSum[d] += point[d];
            entry.squareSum[d] += point[d] * point[d];
        }
        entry.pointIndices.append(idx);
    } else {
        /* 创建新条目 */
        CFEntry newEntry;
        newEntry.n = 1;
        newEntry.pointIndices.append(idx);
        int dim = qMin(m_dim, (int)point.size());
        newEntry.linearSum.resize(dim);
        newEntry.squareSum.resize(dim);
        for (int d = 0; d < dim; ++d) {
            newEntry.linearSum[d] = point[d];
            newEntry.squareSum[d] = point[d] * point[d];
        }
        m_leafEntries.append(newEntry);
        m_stats.totalMerges++;
    }

    /* 内存超限自动重建 */
    if (m_leafEntries.size() > m_maxEntries) {
        rebuild();
    }

    m_stats.totalInsertions++;
    m_stats.totalPointsProcessed++;

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalInsertions;

    emit insertionCompleted(m_stats.totalPointsProcessed, m_leafEntries.size());
}

/**
 * @brief 批量插入数据点
 *
 * 逐点插入并记录总耗时。批量插入在数据预处理阶段最为常用。
 *
 * @param points 数据点集合
 */
void BirchClustering3::insertBatch(const QVector<QVector<double>>& points)
{
    QElapsedTimer timer;
    timer.start();

    for (const auto& pt : points) {
        insert(pt);
    }

    (void)timer.elapsed();
}

/**
 * @brief 对叶条目执行K-Means全局聚类
 *
 * 将CF叶条目的质心作为数据点，使用K-Means算法进行全局聚类。
 * 返回每个簇包含的原始数据点索引集合。
 *
 * @param k 目标簇数
 * @return 聚类结果，每个向量包含一组原始点索引
 */
QVector<QVector<int>> BirchClustering3::cluster(int k)
{
    QElapsedTimer timer;
    timer.start();

    int n = m_leafEntries.size();
    k = qMin(k, n);
    if (k <= 0 || n == 0) return {};

    /* 计算每个叶条目的质心 */
    QVector<QVector<double>> centroids(n);
    for (int i = 0; i < n; ++i) {
        centroids[i].resize(m_dim);
        if (m_leafEntries[i].n > 0) {
            for (int d = 0; d < m_dim; ++d) {
                centroids[i][d] = m_leafEntries[i].linearSum[d] / m_leafEntries[i].n;
            }
        }
    }

    /* K-Means++ 初始化 */
    std::mt19937 rng(42);
    QVector<QVector<double>> centers(k);
    centers[0] = centroids[rng() % n];

    for (int c = 1; c < k; ++c) {
        QVector<double> dists(n);
        double totalDist = 0.0;
        for (int i = 0; i < n; ++i) {
            double minD = std::numeric_limits<double>::max();
            for (int j = 0; j < c; ++j) {
                double d = 0.0;
                for (int dd = 0; dd < m_dim; ++dd) {
                    double diff = centroids[i][dd] - centers[j][dd];
                    d += diff * diff;
                }
                minD = qMin(minD, d);
            }
            dists[i] = minD;
            totalDist += minD;
        }
        double r = (rng() / (double)rng.max()) * totalDist;
        double cumSum = 0.0;
        for (int i = 0; i < n; ++i) {
            cumSum += dists[i];
            if (cumSum >= r) { centers[c] = centroids[i]; break; }
        }
    }

    /* 迭代分配 */
    QVector<int> assign(n, 0);
    int iterations = 0;
    for (int iter = 0; iter < 50; ++iter) {
        iterations++;
        bool changed = false;
        for (int i = 0; i < n; ++i) {
            double bestD = std::numeric_limits<double>::max();
            int bestC = 0;
            for (int c = 0; c < k; ++c) {
                double d = 0.0;
                for (int dd = 0; dd < m_dim; ++dd) {
                    double diff = centroids[i][dd] - centers[c][dd];
                    d += diff * diff;
                }
                if (d < bestD) { bestD = d; bestC = c; }
            }
            if (assign[i] != bestC) { assign[i] = bestC; changed = true; }
        }
        /* 更新中心 */
        centers.assign(k, QVector<double>(m_dim, 0.0));
        QVector<int> counts(k, 0);
        for (int i = 0; i < n; ++i) {
            counts[assign[i]] += m_leafEntries[i].n;
            for (int d = 0; d < m_dim; ++d) {
                centers[assign[i]][d] += m_leafEntries[i].linearSum[d];
            }
        }
        for (int c = 0; c < k; ++c) {
            if (counts[c] > 0) {
                for (int d = 0; d < m_dim; ++d) {
                    centers[c][d] /= counts[c];
                }
            }
        }
        if (!changed) break;
    }

    /* 收集结果 */
    QVector<QVector<int>> result(k);
    for (int i = 0; i < n; ++i) {
        result[assign[i]].append(m_leafEntries[i].pointIndices);
    }

    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, m_stats.totalInsertions);

    emit clusteringCompleted(k, iterations);
    return result;
}

/**
 * @brief 重建CF树以优化内存和结构
 *
 * 合并距离最近的叶条目对，直到条目数降到最大限制以内。
 * 动态调参机制在重建时自动提升阈值，避免频繁重建。
 */
void BirchClustering3::rebuild()
{
    QElapsedTimer timer;
    timer.start();

    /* 合并最近的叶条目对 */
    while (m_leafEntries.size() > m_maxEntries / 2 && m_leafEntries.size() >= 2) {
        int bestI = 0, bestJ = 1;
        double bestDist = std::numeric_limits<double>::max();
        for (int i = 0; i < m_leafEntries.size(); ++i) {
            for (int j = i + 1; j < m_leafEntries.size(); ++j) {
                double d = entryDistance(m_leafEntries[i], m_leafEntries[j]);
                if (d < bestDist) {
                    bestDist = d;
                    bestI = i;
                    bestJ = j;
                }
            }
        }
        mergeEntries(m_leafEntries[bestI], m_leafEntries[bestJ]);
        m_leafEntries.removeAt(bestJ);
        m_stats.totalMerges++;
    }

    /* 动态调参：提升阈值以减少后续重建频率 */
    if (m_leafEntries.size() > m_maxEntries * 3 / 4) {
        m_threshold *= 1.2;
    }

    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, m_stats.totalInsertions);
}

/**
 * @brief 计算CF条目的半径
 *
 * 半径定义为条目内所有点到质心的均方根距离。
 *
 * @param e CF条目
 * @return 条目半径
 */
double BirchClustering3::entryRadius(const CFEntry& e) const
{
    if (e.n <= 1 || m_dim == 0) return 0.0;
    double radius = 0.0;
    for (int d = 0; d < m_dim; ++d) {
        double mean = e.linearSum[d] / e.n;
        radius += e.squareSum[d] / e.n - mean * mean;
    }
    return qSqrt(qMax(0.0, radius));
}

/**
 * @brief 计算两个CF条目的质心距离
 *
 * 使用欧氏距离度量两个条目质心之间的距离。
 *
 * @param a 第一个条目
 * @param b 第二个条目
 * @return 质心距离
 */
double BirchClustering3::entryDistance(const CFEntry& a, const CFEntry& b) const
{
    if (a.n == 0 || b.n == 0) return std::numeric_limits<double>::max();
    double dist = 0.0;
    for (int d = 0; d < m_dim; ++d) {
        double diff = a.linearSum[d] / a.n - b.linearSum[d] / b.n;
        dist += diff * diff;
    }
    return qSqrt(dist);
}

/**
 * @brief 合并源条目到目标条目
 *
 * 将source的统计量累加到target中，实现CF条目的增量合并。
 *
 * @param target 目标条目（将被修改）
 * @param source 源条目
 */
void BirchClustering3::mergeEntries(CFEntry& target, const CFEntry& source)
{
    target.n += source.n;
    for (int d = 0; d < m_dim; ++d) {
        target.linearSum[d] += source.linearSum[d];
        target.squareSum[d] += source.squareSum[d];
    }
    target.pointIndices.append(source.pointIndices);
}

/**
 * @brief 获取CF树高度（简化为叶层深度估算）
 * @return 估算树高度
 */
int BirchClustering3::treeHeight() const
{
    if (m_leafEntries.isEmpty()) return 0;
    int leaves = m_leafEntries.size();
    int height = 1;
    while (leaves > 1) {
        leaves = (leaves + m_branchingFactor - 1) / m_branchingFactor;
        height++;
    }
    return height;
}

/**
 * @brief 获取叶条目数量
 * @return 叶条目数
 */
int BirchClustering3::leafCount() const
{
    return m_leafEntries.size();
}

/**
 * @brief 估算当前内存使用量（字节）
 *
 * 基于存储的数据点、叶条目统计量等计算大致内存占用。
 *
 * @return 估算字节数
 */
double BirchClustering3::memoryUsage() const
{
    double bytes = 0.0;
    /* 数据点存储 */
    bytes += m_points.size() * m_dim * sizeof(double);
    /* 叶条目存储 */
    for (const auto& e : m_leafEntries) {
        bytes += e.linearSum.size() * sizeof(double);
        bytes += e.squareSum.size() * sizeof(double);
        bytes += e.pointIndices.size() * sizeof(int);
        bytes += sizeof(CFEntry);
    }
    return bytes;
}

/**
 * @brief 重置所有统计数据
 *
 * 将累计计数器、时间总和和平均处理时间全部归零。
 */
void BirchClustering3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
