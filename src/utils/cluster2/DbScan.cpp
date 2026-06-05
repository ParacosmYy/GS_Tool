/**
 * @file DbScan.cpp
 * @brief DBSCAN密度聚类实现
 */

#include "utils/cluster2/DbScan.h"

#include <QElapsedTimer>
#include <QtMath>

/** @brief 构造函数 @param parent 父对象 */
DbScan::DbScan(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

/** @brief DBSCAN聚类 */
DbScan::ClusterResult DbScan::fit(
    const QVector<QVector<double>>& data,
    double eps, int minPts,
    const DistFunc& distFunc)
{
    QElapsedTimer timer;
    timer.start();

    ClusterResult result;
    int n = data.size();
    if (n == 0) return result;

    result.labels.fill(-1, n);
    int clusterLabel = 0;

    /* 默认欧氏距离 */
    DistFunc dist = distFunc;
    if (!dist) {
        dist = [](const QVector<double>& a, const QVector<double>& b) {
            double sum = 0.0;
            int d = qMin(a.size(), b.size());
            for (int i = 0; i < d; ++i) {
                double diff = a[i] - b[i];
                sum += diff * diff;
            }
            return qSqrt(sum);
        };
    }

    /* 访问标记 */
    QVector<bool> visited(n, false);

    for (int i = 0; i < n; ++i) {
        if (visited[i]) continue;
        visited[i] = true;

        QVector<int> neighbors = rangeQuery(data, i, eps, dist);

        if (neighbors.size() < minPts) {
            /* 标记为噪声(后续可能被改为边界点) */
            result.labels[i] = -1;
            continue;
        }

        /* 开始新簇 */
        result.labels[i] = clusterLabel;
        QVector<int> seedSet = neighbors;

        /* 扩展簇 */
        int idx = 0;
        while (idx < seedSet.size()) {
            int q = seedSet[idx];
            ++idx;

            if (q < 0 || q >= n) continue;

            if (!visited[q]) {
                visited[q] = true;
                QVector<int> qNeighbors = rangeQuery(data, q, eps, dist);
                if (static_cast<int>(qNeighbors.size()) >= minPts) {
                    for (int nn : qNeighbors) {
                        if (!seedSet.contains(nn))
                            seedSet.append(nn);
                    }
                }
            }

            if (result.labels[q] < 0)
                result.labels[q] = clusterLabel;
        }

        ++clusterLabel;
    }

    result.clusterCount = clusterLabel;
    result.noiseCount = 0;
    for (int i = 0; i < n; ++i)
        if (result.labels[i] < 0) ++result.noiseCount;

    double elapsed = static_cast<double>(timer.elapsed());
    m_timeSum += elapsed;
    ++m_stats.totalClusterings;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalClusterings);

    emit clusteringCompleted(result.clusterCount, result.noiseCount);
    return result;
}

/** @brief 范围查询 */
QVector<int> DbScan::rangeQuery(const QVector<QVector<double>>& data,
                                 int pointIdx, double eps,
                                 const DistFunc& distFunc) const
{
    QVector<int> neighbors;
    int n = data.size();
    double epsSq = eps * eps;

    for (int i = 0; i < n; ++i) {
        double d = distFunc(data[pointIdx], data[i]);
        if (d <= eps)
            neighbors.append(i);
    }
    return neighbors;
}

/** @brief 重置统计 */
void DbScan::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
