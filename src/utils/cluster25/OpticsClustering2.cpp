/**
 * @file OpticsClustering2.cpp
 * @brief OPTICS聚类增强实现 — 可达性图/簇提取/核心距离/多密度
 */

#include "utils/cluster25/OpticsClustering2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
OpticsClustering2::OpticsClustering2(QObject* parent)
    : QObject(parent)
{
}

/** @brief 计算两点欧氏距离 */
static double euclideanDist(const OpticsClustering2::Point& a,
                            const OpticsClustering2::Point& b)
{
    double sum = 0.0;
    int dims = qMin(a.coordinates.size(), b.coordinates.size());
    for (int i = 0; i < dims; ++i) {
        double d = a.coordinates[i] - b.coordinates[i];
        sum += d * d;
    }
    return qSqrt(sum);
}

/** @brief 执行OPTICS排序 @param points 输入点集 @param epsilon 邻域半径 @param minPts 最小点数 @return 排序结果 */
QList<OpticsClustering2::Point> OpticsClustering2::runOptics(
    const QList<Point>& points, double epsilon, int minPts)
{
    QElapsedTimer timer;
    timer.start();

    QList<Point> pts = points;
    int n = pts.size();
    if (n == 0) return pts;

    /* 初始化状态 */
    QVector<bool> processed(n, false);
    QList<int> orderList;

    for (int i = 0; i < n; ++i) {
        if (processed[i]) continue;

        processed[i] = true;
        orderList.append(i);
        pts[i].order = orderList.size() - 1;

        /* 计算核心距离 */
        double cDist = computeCoreDistance(pts, i, epsilon, minPts);
        pts[i].coreDist = cDist;

        if (cDist < 0.0) continue; /* 非核心点，无种子 */

        /* 种子列表 */
        QVector<int> neighbors = findNeighbors(pts, i, epsilon);
        QList<int> seeds;
        updateSeeds(pts, neighbors, i, cDist, seeds, processed);

        /* 迭代处理种子 */
        while (!seeds.isEmpty()) {
            /* 找最小可达距离的种子 */
            int bestIdx = 0;
            double bestReach = pts[seeds[0]].reachability;
            for (int s = 1; s < seeds.size(); ++s) {
                if (pts[seeds[s]].reachability < bestReach) {
                    bestReach = pts[seeds[s]].reachability;
                    bestIdx = s;
                }
            }

            int current = seeds.takeAt(bestIdx);
            processed[current] = true;
            orderList.append(current);
            pts[current].order = orderList.size() - 1;

            double curCore = computeCoreDistance(pts, current, epsilon, minPts);
            pts[current].coreDist = curCore;

            if (curCore >= 0.0) {
                QVector<int> curNeighbors = findNeighbors(pts, current, epsilon);
                updateSeeds(pts, curNeighbors, current, curCore,
                            seeds, processed);
            }
        }
    }

    /* 更新统计 */
    double elapsed = timer.elapsed();
    m_stats.totalClusteringRuns++;
    m_stats.totalPointsProcessed += n;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalClusteringRuns);

    emit clusteringComplete(0, n);
    return pts;
}

/** @brief 更新种子列表的可达距离 @param points 点集 @param neighbors 邻居索引 @param centerIdx 中心点 @param coreDist 核心距离 @param seeds 种子列表 @param processed 已处理标记 */
void OpticsClustering2::updateSeeds(QList<Point>& points,
                                    const QVector<int>& neighbors,
                                    int centerIdx, double coreDist,
                                    QList<int>& seeds,
                                    QVector<bool>& processed)
{
    for (int nbIdx : neighbors) {
        if (processed[nbIdx]) continue;

        double dist = euclideanDist(points[centerIdx], points[nbIdx]);
        double newReach = qMax(coreDist, dist);

        if (points[nbIdx].reachability < 0.0) {
            /* 首次到达该点 */
            points[nbIdx].reachability = newReach;
            seeds.append(nbIdx);
        } else if (newReach < points[nbIdx].reachability) {
            /* 更新更小的可达距离 */
            points[nbIdx].reachability = newReach;
        }
    }
}

/** @brief 查找邻域 @param points 点集 @param idx 当前点 @param epsilon 半径 @return 邻居索引 */
QVector<int> OpticsClustering2::findNeighbors(const QList<Point>& points,
                                               int idx, double epsilon) const
{
    QVector<int> neighbors;
    int n = points.size();
    for (int i = 0; i < n; ++i) {
        if (i == idx) continue;
        if (euclideanDist(points[idx], points[i]) <= epsilon) {
            neighbors.append(i);
        }
    }
    return neighbors;
}

/** @brief 计算核心距离 @param points 点集 @param idx 当前点 @param epsilon 半径 @param minPts 最小点数 @return 核心距离(不足返回-1) */
double OpticsClustering2::computeCoreDistance(const QList<Point>& points,
                                              int idx, double epsilon,
                                              int minPts) const
{
    QVector<double> distances;
    int n = points.size();
    distances.reserve(n - 1);

    for (int i = 0; i < n; ++i) {
        if (i == idx) continue;
        double d = euclideanDist(points[idx], points[i]);
        if (d <= epsilon) {
            distances.append(d);
        }
    }

    if (distances.size() < minPts) return -1.0;

    /* 第minPts-1小的距离即为核心距离 */
    std::nth_element(distances.begin(),
                     distances.begin() + minPts - 1,
                     distances.end());
    return distances[minPts - 1];
}

/** @brief Xi方法提取簇 @param orderedPoints OPTICS排序结果 @param xi 下坡阈值 @return 簇列表 */
QList<OpticsClustering2::Cluster> OpticsClustering2::extractClustersXi(
    const QList<Point>& orderedPoints, double xi) const
{
    QList<Cluster> clusters;
    if (orderedPoints.isEmpty()) return clusters;

    int n = orderedPoints.size();
    int clusterId = 0;
    int i = 0;

    while (i < n) {
        /* 寻找下坡起点: 可达距离开始下降 */
        while (i < n && orderedPoints[i].reachability < 0.0) ++i;
        if (i >= n) break;

        double startReach = orderedPoints[i].reachability;
        int startIdx = i;

        /* 扫描下坡区域 */
        double minReach = startReach;
        int minIdx = i;
        while (i < n) {
            double r = orderedPoints[i].reachability;
            if (r < 0.0) { ++i; continue; }
            if (r < minReach) {
                minReach = r;
                minIdx = i;
            }
            /* 检测上升幅度超过阈值则结束簇 */
            if (r > startReach * (1.0 - xi)) {
                break;
            }
            ++i;
        }

        int endIdx = (i < n) ? i : n;

        /* 最少3个点才算有效簇 */
        if (endIdx - startIdx >= 3) {
            Cluster c;
            c.id = clusterId++;
            double reachSum = 0.0;
            int reachCount = 0;
            for (int j = startIdx; j < endIdx; ++j) {
                c.pointIndices.append(j);
                if (orderedPoints[j].reachability > 0.0) {
                    reachSum += orderedPoints[j].reachability;
                    ++reachCount;
                }
            }
            c.avgReachability = (reachCount > 0)
                ? reachSum / reachCount : 0.0;
            clusters.append(c);
        }
    }

    /* 统计 */
    const_cast<OpticsClustering2*>(this)->m_stats.totalClustersFound
        += clusters.size();

    return clusters;
}

/** @brief 可达性图数据 @param orderedPoints 排序结果 @return (序号, 可达距离) */
QVector<QPair<int, double>> OpticsClustering2::reachabilityPlot(
    const QList<Point>& orderedPoints) const
{
    QVector<QPair<int, double>> plot;
    plot.reserve(orderedPoints.size());
    for (int i = 0; i < orderedPoints.size(); ++i) {
        double r = orderedPoints[i].reachability;
        plot.append({i, (r < 0.0) ? 0.0 : r});
    }
    return plot;
}

/** @brief 重置统计 */
void OpticsClustering2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
