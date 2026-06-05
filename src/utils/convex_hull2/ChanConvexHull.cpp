/**
 * @file ChanConvexHull.cpp
 * @brief Chan算法凸包实现 — O(n log h)最优二维凸包
 */

#include "utils/convex_hull2/ChanConvexHull.h"

#include <QElapsedTimer>
#include <algorithm>
#include <cmath>

// ============================================================================
// 构造
// ============================================================================

ChanConvexHull::ChanConvexHull(QObject* parent)
    : QObject(parent), m_timeSum(0.0)
{
}

// ============================================================================
// 公开方法
// ============================================================================

QVector<int> ChanConvexHull::compute2d(
    QVector<QPair<double,double>> points)
{
    QElapsedTimer timer;
    timer.start();

    int n = points.size();
    QVector<int> result;

    if (n < 3) {
        for (int i = 0; i < n; ++i) {
            result.append(i);
        }
        ++m_stats.totalComputations;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = (m_stats.totalComputations > 0)
            ? m_timeSum / m_stats.totalComputations : 0.0;
        emit computationCompleted(result.size());
        return result;
    }

    /* 检查是否所有点共线 */
    bool collinear = true;
    for (int i = 2; i < n && collinear; ++i) {
        if (std::abs(cross(points[0], points[1], points[i])) > 1e-12) {
            collinear = false;
        }
    }
    if (collinear) {
        /* 所有点共线: 返回两端点 */
        int minIdx = 0, maxIdx = 0;
        for (int i = 1; i < n; ++i) {
            if (points[i] < points[minIdx]) minIdx = i;
            if (points[i] > points[maxIdx]) maxIdx = i;
        }
        result.append(minIdx);
        if (maxIdx != minIdx) result.append(maxIdx);

        ++m_stats.totalComputations;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = (m_stats.totalComputations > 0)
            ? m_timeSum / m_stats.totalComputations : 0.0;
        emit computationCompleted(result.size());
        return result;
    }

    /* Chan算法: 尝试不同的h猜测值 */
    for (int k = 1; ; ++k) {
        int h = std::min(static_cast<int>(std::pow(2.0, std::pow(2.0, k))),
                         n);

        /* 分成 ceil(n/h) 个子集 */
        int m = (n + h - 1) / h;
        QVector<QVector<int>> subHulls;

        for (int i = 0; i < m; ++i) {
            int start = i * h;
            int end = std::min(start + h, n);
            QVector<int> subIndices;
            subIndices.reserve(end - start);
            for (int j = start; j < end; ++j) {
                subIndices.append(j);
            }
            subHulls.append(grahamScan(points, subIndices));
        }

        /* Jarvis步进法合并 */
        result = jarvisMerge(points, subHulls, h);

        if (result.size() <= h) {
            break; // 成功
        }
    }

    ++m_stats.totalComputations;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalComputations > 0)
        ? m_timeSum / m_stats.totalComputations : 0.0;

    emit computationCompleted(result.size());
    return result;
}

void ChanConvexHull::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

// ============================================================================
// 内部实现
// ============================================================================

double ChanConvexHull::cross(const QPair<double,double>& o,
                              const QPair<double,double>& a,
                              const QPair<double,double>& b)
{
    return (a.first - o.first) * (b.second - o.second)
         - (b.first - o.first) * (a.second - o.second);
}

QVector<int> ChanConvexHull::grahamScan(
    const QVector<QPair<double,double>>& pts,
    const QVector<int>& indices)
{
    if (indices.size() <= 2) return indices;

    /* 找最低点(按y优先，x次之) */
    int pivotIdx = indices[0];
    for (int i = 1; i < indices.size(); ++i) {
        int idx = indices[i];
        if (pts[idx].second < pts[pivotIdx].second ||
            (pts[idx].second == pts[pivotIdx].second &&
             pts[idx].first < pts[pivotIdx].first)) {
            pivotIdx = idx;
        }
    }

    /* 按极角排序 */
    QVector<int> sorted = indices;
    int pivot = pivotIdx;
    std::sort(sorted.begin(), sorted.end(),
        [&pts, pivot, this](int a, int b) {
            double cr = cross(pts[pivot], pts[a], pts[b]);
            if (std::abs(cr) < 1e-12) {
                double da = (pts[a].first - pts[pivot].first) *
                            (pts[a].first - pts[pivot].first) +
                            (pts[a].second - pts[pivot].second) *
                            (pts[a].second - pts[pivot].second);
                double db = (pts[b].first - pts[pivot].first) *
                            (pts[b].first - pts[pivot].first) +
                            (pts[b].second - pts[pivot].second) *
                            (pts[b].second - pts[pivot].second);
                return da < db;
            }
            return cr > 0;
        });

    /* Graham扫描 */
    QVector<int> hull;
    for (int idx : sorted) {
        while (hull.size() >= 2) {
            double cr = cross(pts[hull[hull.size() - 2]],
                              pts[hull[hull.size() - 1]],
                              pts[idx]);
            if (cr <= 1e-12) {
                hull.removeLast();
            } else {
                break;
            }
        }
        hull.append(idx);
    }

    return hull;
}

QVector<int> ChanConvexHull::jarvisMerge(
    const QVector<QPair<double,double>>& pts,
    const QVector<QVector<int>>& subHulls,
    int maxSteps)
{
    /* 找全局最低点 */
    int current = 0;
    for (int i = 1; i < pts.size(); ++i) {
        if (pts[i].second < pts[current].second ||
            (pts[i].second == pts[current].second &&
             pts[i].first < pts[current].first)) {
            current = i;
        }
    }

    QVector<int> hull;
    hull.append(current);

    QPair<double,double> prev = {pts[current].first - 1.0,
                                  pts[current].second};

    for (int step = 0; step < maxSteps; ++step) {
        /* 在每个子凸包上找最右切点 */
        int bestNext = -1;
        double bestCross = -1e30;

        for (const auto& sh : subHulls) {
            if (sh.isEmpty()) continue;

            int localBest = sh[0];
            double localBestCross = -1e30;

            for (int vi : sh) {
                double cr = cross(pts[current], pts[localBest], pts[vi]);
                if (cr < -1e-12) {
                    localBest = vi;
                    localBestCross = cr;
                } else if (std::abs(cr) < 1e-12) {
                    /* 共线: 取较远的点 */
                    double dOld = (pts[localBest].first - pts[current].first) *
                                  (pts[localBest].first - pts[current].first) +
                                  (pts[localBest].second - pts[current].second) *
                                  (pts[localBest].second - pts[current].second);
                    double dNew = (pts[vi].first - pts[current].first) *
                                  (pts[vi].first - pts[current].first) +
                                  (pts[vi].second - pts[current].second) *
                                  (pts[vi].second - pts[current].second);
                    if (dNew > dOld) {
                        localBest = vi;
                    }
                }
            }

            /* 与全局最优比较 */
            if (bestNext == -1) {
                bestNext = localBest;
            } else {
                double cr = cross(pts[current], pts[bestNext], pts[localBest]);
                if (cr < -1e-12) {
                    bestNext = localBest;
                } else if (std::abs(cr) < 1e-12) {
                    double dOld = (pts[bestNext].first - pts[current].first) *
                                  (pts[bestNext].first - pts[current].first) +
                                  (pts[bestNext].second - pts[current].second) *
                                  (pts[bestNext].second - pts[current].second);
                    double dNew = (pts[localBest].first - pts[current].first) *
                                  (pts[localBest].first - pts[current].first) +
                                  (pts[localBest].second - pts[current].second) *
                                  (pts[localBest].second - pts[current].second);
                    if (dNew > dOld) {
                        bestNext = localBest;
                    }
                }
            }
        }

        if (bestNext == -1 || bestNext == current) break;
        if (bestNext == hull[0]) break; // 回到起点

        hull.append(bestNext);
        current = bestNext;
    }

    return hull;
}
