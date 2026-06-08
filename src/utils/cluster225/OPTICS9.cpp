/**
 * @file OPTICS9.cpp
 * @brief OPTICS9 实现
 *
 * 实现OPTICS排序算法：渐进可达性绘图与层次聚类提取。
 */

#include "utils/cluster225/OPTICS9.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

OPTICS9::OPTICS9(QObject *parent) : QObject(parent) {}
OPTICS9::~OPTICS9() = default;

/* ---- Configuration ---- */

void OPTICS9::setParameters(double epsilon, int minPoints)
{
    m_epsilon = qMax(0.001, epsilon);
    m_minPoints = qMax(2, minPoints);
}

/* ---- Euclidean distance ---- */

double OPTICS9::distance(const QVector<double>& a,
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

/* ---- Range query ---- */

QVector<int> OPTICS9::rangeQuery(int idx) const
{
    QVector<int> neighbors;
    for (int i = 0; i < m_data.size(); ++i) {
        if (distance(m_data[idx], m_data[i]) <= m_epsilon)
            neighbors.append(i);
    }
    return neighbors;
}

/* ---- Core distance ---- */

double OPTICS9::coreDistance(int idx, const QVector<int>& neighbors) const
{
    if (neighbors.size() < m_minPoints) return -1.0;
    // Distance to the minPoints-th nearest neighbor
    QVector<double> dists;
    for (int n : neighbors) {
        if (n != idx)
            dists.append(distance(m_data[idx], m_data[n]));
    }
    std::sort(dists.begin(), dists.end());
    if (dists.size() < static_cast<int>(m_minPoints - 1)) return -1.0;
    return dists[m_minPoints - 2];
}

/* ---- Update seeds ---- */

void OPTICS9::updateSeeds(int idx, const QVector<int>& neighbors,
                           QVector<double>& reachDist,
                           QVector<bool>& processed,
                           QVector<int>& /*predecessor*/)
{
    double cDist = coreDistance(idx, neighbors);
    if (cDist < 0) return;

    for (int n : neighbors) {
        if (processed[n]) continue;
        double newReach = qMax(cDist, distance(m_data[idx], m_data[n]));
        if (reachDist[n] < 0 || newReach < reachDist[n])
            reachDist[n] = newReach;
    }
}

/* ---- Fit (main OPTICS loop) ---- */

QVector<OPTICS9::OrderEntry> OPTICS9::fit(
    const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    if (n < 2) return {};

    m_data = data;
    m_ordering.clear();

    QVector<bool> processed(n, false);
    QVector<double> reachDist(n, -1.0);
    QVector<int> predecessor(n, -1);

    int coreCount = 0;

    for (int i = 0; i < n; ++i) {
        if (processed[i]) continue;
        processed[i] = true;

        QVector<int> neighbors = rangeQuery(i);
        double cDist = coreDistance(i, neighbors);

        OrderEntry entry;
        entry.pointIndex = i;
        entry.reachability = reachDist[i];
        entry.coreDistance = cDist;
        m_ordering.append(entry);

        if (cDist >= 0) {
            coreCount++;
            updateSeeds(i, neighbors, reachDist, processed, predecessor);

            // Priority queue simulation via sorted list
            while (true) {
                // Find unprocessed with smallest reachability
                int best = -1;
                double bestDist = std::numeric_limits<double>::max();
                for (int j = 0; j < n; ++j) {
                    if (!processed[j] && reachDist[j] >= 0 && reachDist[j] < bestDist) {
                        bestDist = reachDist[j];
                        best = j;
                    }
                }
                if (best < 0) break;

                processed[best] = true;
                QVector<int> nbs = rangeQuery(best);
                double cd = coreDistance(best, nbs);

                OrderEntry e;
                e.pointIndex = best;
                e.reachability = reachDist[best];
                e.coreDistance = cd;
                m_ordering.append(e);

                if (cd >= 0) {
                    coreCount++;
                    updateSeeds(best, nbs, reachDist, processed, predecessor);
                }
            }
        }
    }

    m_stats.numPoints = n;
    m_stats.numCorePoints = coreCount;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit orderingCompleted(n, coreCount, timer.elapsed());
    return m_ordering;
}

/* ---- Extract clusters from reachability plot ---- */

QVector<OPTICS9::Cluster> OPTICS9::extractClusters(double xiThreshold) const
{
    QVector<Cluster> clusters;
    int n = m_ordering.size();
    if (n < 2) return clusters;

    // Scan for steep down areas followed by steep up areas
    bool inCluster = false;
    int clusterStart = 0;
    double clusterStartReach = 0.0;

    for (int i = 0; i < n; ++i) {
        double r = (m_ordering[i].reachability < 0)
                       ? std::numeric_limits<double>::max()
                       : m_ordering[i].reachability;

        if (!inCluster) {
            // Start of steep-down region
            if (i + 1 < n) {
                double rNext = (m_ordering[i + 1].reachability < 0)
                                   ? std::numeric_limits<double>::max()
                                   : m_ordering[i + 1].reachability;
                if (rNext < r * (1.0 - xiThreshold)) {
                    inCluster = true;
                    clusterStart = i;
                    clusterStartReach = r;
                }
            }
        } else {
            // End of cluster: reachability rises above start
            if (r > clusterStartReach * (1.0 + xiThreshold) || i == n - 1) {
                Cluster cl;
                for (int j = clusterStart; j <= i; ++j)
                    cl.pointIndices.append(m_ordering[j].pointIndex);
                cl.startReach = clusterStartReach;
                cl.endReach = r;
                clusters.append(cl);
                inCluster = false;
            }
        }
    }

    const_cast<OPTICS9*>(this)->m_stats.numClustersExtracted = clusters.size();
    emit const_cast<OPTICS9*>(this)->clustersExtracted(
        clusters.size(), xiThreshold);
    return clusters;
}

/* ---- Reachability plot ---- */

QVector<double> OPTICS9::reachabilityPlot() const
{
    QVector<double> plot;
    plot.reserve(m_ordering.size());
    for (const auto& e : m_ordering)
        plot.append(e.reachability);
    return plot;
}

/* ---- Reset ---- */

void OPTICS9::resetStatistics()
{
    m_ordering.clear();
    m_data.clear();
    m_stats = Stats{};
    m_timeSum = 0.0;
}
