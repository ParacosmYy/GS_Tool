/**
 * @file OpticsClustering.cpp
 * @brief OPTICS排序聚类算法实现 — 基于可达性图
 */

#include "utils/cluster9/OpticsClustering.h"

#include <QElapsedTimer>

#include <cmath>
#include <algorithm>
#include <limits>

/** @brief 构造函数 @param epsilon 邻域半径 @param minPoints 核心点最小邻居数 @param parent 父对象 */
OpticsClustering::OpticsClustering(double epsilon, int minPoints, QObject* parent)
    : QObject(parent)
    , m_epsilon(epsilon)
    , m_minPoints(minPoints)
{
}

/** @brief 设置数据点 @param points 二维数据 */
void OpticsClustering::setData(const QVector<QVector<double>>& points)
{
    m_points.clear();
    m_points.reserve(points.size());
    for (int i = 0; i < points.size(); ++i) {
        Point p;
        p.coords = points[i];
        p.index = i;
        m_points.append(p);
    }
}

/** @brief 两点间欧氏距离 */
double OpticsClustering::distance(int i, int j) const
{
    const auto& a = m_points[i].coords;
    const auto& b = m_points[j].coords;
    double sum = 0.0;
    int dim = std::min(a.size(), b.size());
    for (int d = 0; d < dim; ++d) {
        double diff = a[d] - b[d];
        sum += diff * diff;
    }
    return std::sqrt(sum);
}

/** @brief 范围查询(返回epsilon邻域内的点) @param pointIdx 中心点索引 @return 邻域列表 */
QList<QPair<int, double>> OpticsClustering::rangeQuery(int pointIdx) const
{
    QList<QPair<int, double>> neighbors;
    for (int i = 0; i < m_points.size(); ++i) {
        if (i == pointIdx) continue;
        double dist = distance(pointIdx, i);
        if (dist <= m_epsilon) {
            neighbors.append(qMakePair(i, dist));
        }
    }
    /* 包含自身 */
    neighbors.append(qMakePair(pointIdx, 0.0));
    return neighbors;
}

/** @brief 计算核心距离 @param pointIdx 点索引 @return 核心距离(-1表示非核心点) */
double OpticsClustering::coreDistance(int pointIdx) const
{
    QList<double> dists;
    for (int i = 0; i < m_points.size(); ++i) {
        if (i == pointIdx) continue;
        double d = distance(pointIdx, i);
        if (d <= m_epsilon) dists.append(d);
    }
    if (dists.size() < m_minPoints) return -1.0;
    std::sort(dists.begin(), dists.end());
    return dists[m_minPoints - 1];
}

/** @brief 更新种子列表中邻居的可达距离 */
void OpticsClustering::updateSeeds(
    int pointIdx, const QList<QPair<int, double>>& neighbors,
    QVector<double>& reachDist, QVector<bool>& processed,
    QVector<int>& predecessor,
    QList<QPair<double, int>>& seeds) const
{
    double coreDist = coreDistance(pointIdx);

    for (const auto& nb : neighbors) {
        int nbIdx = nb.first;
        double nbDist = nb.second;
        if (processed[nbIdx]) continue;

        double newReachDist = (coreDist < 0.0)
            ? nbDist : std::max(coreDist, nbDist);

        if (reachDist[nbIdx] < 0.0 || newReachDist < reachDist[nbIdx]) {
            reachDist[nbIdx] = newReachDist;
            predecessor[nbIdx] = pointIdx;

            /* 更新或插入到seeds(优先队列) */
            bool found = false;
            for (auto& s : seeds) {
                if (s.second == nbIdx) {
                    s.first = newReachDist;
                    found = true;
                    break;
                }
            }
            if (!found) {
                seeds.append(qMakePair(newReachDist, nbIdx));
            }
        }
    }
}

/** @brief 执行OPTICS排序 @return 排序结果 */
QVector<OpticsClustering::OrderPoint> OpticsClustering::computeOrdering()
{
    QElapsedTimer timer;
    timer.start();

    int n = m_points.size();
    QVector<OrderPoint> ordering;
    ordering.reserve(n);

    QVector<bool> processed(n, false);
    QVector<double> reachDist(n, -1.0);
    QVector<double> coreDist(n, -1.0);
    QVector<int> predecessor(n, -1);

    for (int i = 0; i < n; ++i) {
        if (processed[i]) continue;

        /* 处理当前点 */
        processed[i] = true;
        coreDist[i] = coreDistance(i);

        OrderPoint op;
        op.index = i;
        op.reachabilityDist = reachDist[i];
        op.coreDist = coreDist[i];
        ordering.append(op);

        /* 如果是核心点，扩展邻域 */
        if (coreDist[i] >= 0.0) {
            QList<QPair<double, int>> seeds;
            auto neighbors = rangeQuery(i);
            ++m_stats.totalNeighborsQueried;

            updateSeeds(i, neighbors, reachDist, processed, predecessor, seeds);

            while (!seeds.isEmpty()) {
                /* 选择可达距离最小的种子 */
                int bestIdx = 0;
                for (int j = 1; j < seeds.size(); ++j) {
                    if (seeds[j].first < seeds[bestIdx].first) bestIdx = j;
                }
                int curIdx = seeds[bestIdx].first < 0 ? seeds[0].second : seeds[bestIdx].second;
                seeds.removeAt(bestIdx);

                if (processed[curIdx]) continue;
                processed[curIdx] = true;
                coreDist[curIdx] = coreDistance(curIdx);

                OrderPoint op2;
                op2.index = curIdx;
                op2.reachabilityDist = reachDist[curIdx];
                op2.coreDist = coreDist[curIdx];
                ordering.append(op2);

                if (coreDist[curIdx] >= 0.0) {
                    auto nb2 = rangeQuery(curIdx);
                    ++m_stats.totalNeighborsQueried;
                    updateSeeds(curIdx, nb2, reachDist, processed, predecessor, seeds);
                }
            }
        }
    }

    m_stats.totalPointsProcessed += n;
    ++m_stats.totalClusterings;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalClusterings;

    emit orderingCompleted(n, 0);
    return ordering;
}

/** @brief 从可达性图提取聚类(陡峭阈值法) @param ordering OPTICS排序 @param xi 陡峭阈值 @return 聚类簇列表 */
QList<OpticsClustering::Cluster> OpticsClustering::extractClusters(
    const QVector<OrderPoint>& ordering, double xi) const
{
    QList<Cluster> clusters;
    if (ordering.size() < 2) return clusters;

    /* 扫描可达性图: 陡峭上升/下降的边界划分簇 */
    enum State { Outside, InCluster };
    State state = Outside;
    int clusterStart = 0;

    for (int i = 1; i < ordering.size(); ++i) {
        double prev = ordering[i - 1].reachabilityDist;
        double curr = ordering[i].reachabilityDist;

        if (state == Outside) {
            /* 陡峭下降 → 簇开始 */
            if (prev > 0.0 && curr >= 0.0 && prev > curr * (1.0 + xi)) {
                clusterStart = i;
                state = InCluster;
            }
        } else {
            /* 陡峭上升 → 簇结束 */
            if (curr < 0.0 || (prev >= 0.0 && curr > prev * (1.0 + xi))) {
                Cluster c;
                for (int j = clusterStart; j < i; ++j) {
                    c.pointIndices.append(ordering[j].index);
                }
                c.coreDistance = ordering[clusterStart].coreDist;
                if (c.pointIndices.size() >= m_minPoints) {
                    clusters.append(c);
                }
                state = Outside;
            }
        }
    }

    /* 处理最后一个簇 */
    if (state == InCluster) {
        Cluster c;
        for (int j = clusterStart; j < ordering.size(); ++j) {
            c.pointIndices.append(ordering[j].index);
        }
        c.coreDistance = ordering[clusterStart].coreDist;
        if (c.pointIndices.size() >= m_minPoints) {
            clusters.append(c);
        }
    }

    return clusters;
}

/** @brief 基于距离阈值提取聚类 @param ordering OPTICS排序 @param distanceThreshold 距离阈值 @return 聚类簇列表 */
QList<OpticsClustering::Cluster> OpticsClustering::extractClustersByThreshold(
    const QVector<OrderPoint>& ordering, double distanceThreshold) const
{
    QList<Cluster> clusters;
    if (ordering.isEmpty()) return clusters;

    Cluster current;
    for (int i = 0; i < ordering.size(); ++i) {
        if (ordering[i].reachabilityDist > distanceThreshold || i == 0) {
            /* 开始新簇 */
            if (!current.pointIndices.isEmpty()) {
                clusters.append(current);
            }
            current = Cluster();
        }
        current.pointIndices.append(ordering[i].index);
        if (ordering[i].coreDist >= 0.0) {
            current.coreDistance = ordering[i].coreDist;
        }
    }
    if (!current.pointIndices.isEmpty()) {
        clusters.append(current);
    }

    return clusters;
}

/** @brief 获取可达性距离数组 @param ordering OPTICS排序 @return 可达性距离数组 */
QVector<double> OpticsClustering::reachabilityPlot(
    const QVector<OrderPoint>& ordering) const
{
    QVector<double> plot;
    plot.reserve(ordering.size());
    for (const auto& op : ordering) {
        plot.append(op.reachabilityDist);
    }
    return plot;
}

/** @brief 设置参数 @param epsilon 新半径 @param minPoints 新最小点数 */
void OpticsClustering::setParameters(double epsilon, int minPoints)
{
    m_epsilon = epsilon;
    m_minPoints = minPoints;
}

/** @brief 重置统计 */
void OpticsClustering::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
