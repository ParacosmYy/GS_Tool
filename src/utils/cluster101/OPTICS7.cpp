#include "OPTICS7.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @file OPTICS7.cpp
 * @brief OPTICS聚类算法实现
 *
 * OPTICS(Ordering Points To Identify the Clustering Structure):
 * 生成按可达距离排序的点序列，可通过可达距离图提取不同密度的簇。
 * 核心距离: 使点成为核心点的最小邻域半径
 * 可达距离: max(核心距离(p), dist(p,q))
 */

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父QObject对象指针
 */
OPTICS7::OPTICS7(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置邻域半径上限
 * @param epsilon 最大邻域半径
 */
void OPTICS7::setEpsilon(double epsilon)
{
    m_epsilon = qMax(0.001, epsilon);
}

/**
 * @brief 设置最小邻域点数
 * @param minPts 成为核心点所需的最少邻域点数
 */
void OPTICS7::setMinPts(int minPts)
{
    m_minPts = qMax(2, minPts);
}

/**
 * @brief 计算欧氏距离
 */
static double eucDist(const QVector<double>& a, const QVector<double>& b)
{
    double d = 0.0;
    for (int i = 0; i < a.size(); ++i) d += (a[i] - b[i]) * (a[i] - b[i]);
    return std::sqrt(d);
}

/**
 * @brief 对输入数据执行OPTICS聚类
 *
 * 算法流程:
 * 1. 对每个未处理点计算核心距离
 * 2. 如果是核心点，更新邻居的可达距离
 * 3. 按可达距离从小到大处理候选点
 * 4. 输出有序序列(点索引, 可达距离)
 *
 * @param data 输入数据矩阵
 * @return (点索引, 可达距离)有序序列
 */
QVector<QPair<int, double>> OPTICS7::fit(const QVector<QVector<double>>& data)
{
    if (data.isEmpty()) return {};

    QElapsedTimer timer;
    timer.start();

    const int n = data.size();

    // 预计算所有距离
    QVector<QVector<double>> dist(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            dist[i][j] = eucDist(data[i], data[j]);
            dist[j][i] = dist[i][j];
        }
    }

    // 查找每个点的邻居
    auto getNeighbors = [&](int p) -> QVector<int> {
        QVector<int> neighbors;
        for (int i = 0; i < n; ++i) {
            if (i != p && dist[p][i] <= m_epsilon) {
                neighbors.append(i);
            }
        }
        return neighbors;
    };

    // 计算核心距离
    auto coreDistance = [&](int p, const QVector<int>& neighbors) -> double {
        if (neighbors.size() < m_minPts) return 1e18;
        QVector<double> dists;
        for (int nb : neighbors) dists.append(dist[p][nb]);
        std::sort(dists.begin(), dists.end());
        return dists[m_minPts - 1];
    };

    QVector<bool> processed(n, false);
    QVector<double> reachDist(n, 1e18);
    QVector<QPair<int, double>> ordering;
    ordering.reserve(n);

    for (int i = 0; i < n; ++i) {
        if (processed[i]) continue;

        processed[i] = true;
        QVector<int> neighbors = getNeighbors(i);
        double cd = coreDistance(i, neighbors);

        ordering.append(qMakePair(i, 1e18)); // 未定义可达距离

        if (cd < 1e18) {
            // 更新邻居的可达距离
            QVector<QPair<double, int>> seeds;
            for (int nb : neighbors) {
                if (!processed[nb]) {
                    double newReachDist = qMax(cd, dist[i][nb]);
                    reachDist[nb] = qMin(reachDist[nb], newReachDist);
                    seeds.append(qMakePair(reachDist[nb], nb));
                }
            }

            // 按可达距离排序处理
            std::sort(seeds.begin(), seeds.end());

            for (auto& seed : seeds) {
                int q = seed.second;
                if (processed[q]) continue;

                processed[q] = true;
                QVector<int> qNeighbors = getNeighbors(q);
                double qCD = coreDistance(q, qNeighbors);

                ordering.append(qMakePair(q, reachDist[q]));

                if (qCD < 1e18) {
                    for (int nb : qNeighbors) {
                        if (!processed[nb]) {
                            double newReachDist = qMax(qCD, dist[q][nb]);
                            reachDist[nb] = qMin(reachDist[nb], newReachDist);
                        }
                    }
                }
            }
        }
    }

    m_stats.totalClustered += n;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, m_stats.totalClustered / n);

    emit clusteringCompleted(ordering.size());
    return ordering;
}

/**
 * @brief 重置所有统计信息
 */
void OPTICS7::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
