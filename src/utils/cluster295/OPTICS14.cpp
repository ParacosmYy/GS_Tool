/**
 * @file OPTICS14.cpp
 * @brief OPTICS14 实现
 *
 * 实现OPTICS聚类：epsilon有界可达性与陡降/陡升区域扫描提取变密度层次聚类。
 */

#include "utils/cluster295/OPTICS14.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

OPTICS14::OPTICS14(QObject *parent)
    : QObject(parent) {}

OPTICS14::~OPTICS14() = default;

/* ---- Configuration ---- */

void OPTICS14::setEpsilon(double eps) { m_epsilon = qBound(0.01, eps, 1e6); }
void OPTICS14::setMinPoints(int minPts) { m_minPts = qBound(2, minPts, 10000); }

/* ---- Core distance: distance to the m_minPts-th nearest neighbor ---- */

double OPTICS14::coreDistance(const QVector<QVector<double>>& dist, int p) const
{
    int n = dist.size();
    if (p < 0 || p >= n) return 1e300;

    // Collect distances to all neighbors
    QVector<double> distances;
    distances.reserve(n - 1);
    for (int i = 0; i < n; ++i) {
        if (i != p && dist[p][i] <= m_epsilon)
            distances.append(dist[p][i]);
    }

    if (distances.size() < m_minPts - 1) return 1e300;

    // Sort and pick the (minPts-1)-th smallest distance
    std::sort(distances.begin(), distances.end());
    return distances[m_minPts - 2];
}

/* ---- Epsilon-neighborhood query ---- */

QVector<int> OPTICS14::regionQuery(const QVector<QVector<double>>& dist, int p) const
{
    QVector<int> neighbors;
    int n = dist.size();
    for (int i = 0; i < n; ++i) {
        if (i != p && dist[p][i] <= m_epsilon)
            neighbors.append(i);
    }
    return neighbors;
}

/* ---- Update reachability for seeds ---- */

void OPTICS14::update(const QVector<QVector<double>>& dist, int p,
                       const QVector<int>& neighbors,
                       QVector<PointInfo>& points,
                       QVector<int>& seeds) const
{
    double coredist = points[p].coreDist;
    for (int o : neighbors) {
        if (points[o].processed) continue;
        double newReach = qMax(coredist, dist[p][o]);
        if (newReach < points[o].reachabilityDist) {
            points[o].reachabilityDist = newReach;
            // Insert into seeds if not already present
            bool found = false;
            for (int s : seeds) {
                if (s == o) { found = true; break; }
            }
            if (!found) seeds.append(o);
        }
    }
}

/* ---- Main OPTICS fit ---- */

OPTICS14::OPTICSResult OPTICS14::fit(const QVector<QVector<double>>& distanceMatrix)
{
    QElapsedTimer timer;
    timer.start();

    int n = distanceMatrix.size();
    OPTICSResult result;

    if (n == 0) return result;

    // Initialize all points
    QVector<PointInfo> points(n);
    for (int i = 0; i < n; ++i) {
        points[i].index = i;
        points[i].reachabilityDist = 1e300;
        points[i].coreDist = 1e300;
        points[i].processed = false;
    }

    QVector<PointInfo> ordered;

    for (int i = 0; i < n; ++i) {
        if (points[i].processed) continue;

        // Mark as processed
        points[i].processed = true;
        points[i].coreDist = coreDistance(distanceMatrix, i);
        ordered.append(points[i]);

        // If core object, expand cluster order
        if (points[i].coreDist < 1e300) {
            QVector<int> seeds;
            auto neighbors = regionQuery(distanceMatrix, i);
            update(distanceMatrix, i, neighbors, points, seeds);

            while (!seeds.isEmpty()) {
                // Pick seed with smallest reachability distance
                int bestIdx = 0;
                for (int j = 1; j < seeds.size(); ++j) {
                    if (points[seeds[j]].reachabilityDist <
                        points[seeds[bestIdx]].reachabilityDist)
                        bestIdx = j;
                }

                int c = seeds[bestIdx];
                seeds.remove(bestIdx);

                points[c].processed = true;
                points[c].coreDist = coreDistance(distanceMatrix, c);
                ordered.append(points[c]);

                if (points[c].coreDist < 1e300) {
                    auto cNeighbors = regionQuery(distanceMatrix, c);
                    update(distanceMatrix, c, cNeighbors, points, seeds);
                }
            }
        }
    }

    result.orderedPoints = ordered;
    result.clusters = extractClusters(ordered, 0.05);
    result.numClusters = result.clusters.size();

    double elapsed = timer.elapsed();
    m_stats.numPoints = n;
    m_stats.totalFits++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFits;

    emit fitDone(n, result.numClusters, elapsed);
    return result;
}

/* ---- Steep-down area detection ---- */

int OPTICS14::steepDownArea(const QVector<PointInfo>& ordered,
                             int start, double xi) const
{
    int i = start;
    int n = ordered.size();
    while (i + 1 < n &&
           ordered[i + 1].reachabilityDist <= ordered[i].reachabilityDist * (1.0 - xi))
        i++;
    return i;
}

/* ---- Steep-up area detection ---- */

int OPTICS14::steepUpArea(const QVector<PointInfo>& ordered,
                           int start, double xi) const
{
    int i = start;
    int n = ordered.size();
    while (i + 1 < n &&
           ordered[i + 1].reachabilityDist >= ordered[i].reachabilityDist * (1.0 + xi))
        i++;
    return i;
}

/* ---- Extract clusters using steep-down/up area scanning ---- */

QVector<OPTICS14::Cluster> OPTICS14::extractClusters(
    const QVector<PointInfo>& ordered, double xi) const
{
    QVector<Cluster> clusters;
    int n = ordered.size();
    if (n < 2) return clusters;

    int i = 0;
    while (i < n - 1) {
        // Look for steep-down area
        int sdEnd = steepDownArea(ordered, i, xi);
        if (sdEnd > i) {
            // Found steep-down, look for matching steep-up
            int j = sdEnd + 1;
            while (j < n - 1) {
                int suEnd = steepUpArea(ordered, j, xi);
                if (suEnd > j) {
                    // Steep-down/up pair forms a cluster
                    Cluster cl;
                    for (int k = i; k <= suEnd + 1 && k < n; ++k) {
                        cl.pointIndices.append(ordered[k].index);
                        cl.minReachability = (k == i)
                            ? ordered[k].reachabilityDist
                            : qMin(cl.minReachability, ordered[k].reachabilityDist);
                        cl.maxReachability = qMax(cl.maxReachability,
                                                   ordered[k].reachabilityDist);
                    }
                    if (cl.pointIndices.size() >= m_minPts)
                        clusters.append(cl);
                    j = suEnd + 1;
                    break;
                }
                j++;
            }
            i = sdEnd + 1;
        } else {
            i++;
        }
    }
    return clusters;
}

/* ---- Reset ---- */

void OPTICS14::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
