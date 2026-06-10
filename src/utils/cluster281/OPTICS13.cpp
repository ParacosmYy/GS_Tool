/**
 * @file OPTICS13.cpp
 * @brief OPTICS13 实现
 *
 * 实现OPTICS聚类：增强可达度聚类提取与陡度评估的任意形状聚类边界。
 */

#include "utils/cluster281/OPTICS13.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

OPTICS13::OPTICS13(QObject *parent)
    : QObject(parent) {}

OPTICS13::~OPTICS13() = default;

/* ---- Configuration ---- */

void OPTICS13::setEpsilon(double eps) { m_eps = qMax(1e-6, eps); }
void OPTICS13::setMinPoints(int minPts) { m_minPts = qBound(2, minPts, 500); }
void OPTICS13::setSteepnessThreshold(double xi) { m_xi = qBound(0.01, xi, 0.5); }

/* ---- Euclidean distance ---- */

double OPTICS13::euclideanDist(const QVector<double>& a,
                                const QVector<double>& b) const
{
    double sum = 0.0;
    int d = qMin(a.size(), b.size());
    for (int i = 0; i < d; ++i) {
        double diff = a[i] - b[i];
        sum += diff * diff;
    }
    return qSqrt(sum);
}

/* ---- Epsilon-neighborhood ---- */

QVector<int> OPTICS13::findNeighbors(int idx,
                                      const QVector<QVector<double>>& data) const
{
    QVector<int> nbrs;
    int n = data.size();
    for (int i = 0; i < n; ++i) {
        if (i == idx) continue;
        if (euclideanDist(data[idx], data[i]) <= m_eps)
            nbrs.append(i);
    }
    return nbrs;
}

/* ---- Core distance ---- */

double OPTICS13::coreDist(int idx, const QVector<QVector<double>>& data,
                           const QVector<int>& neighbors) const
{
    if (neighbors.size() < m_minPts - 1) return 1e30;

    // Sort distances to find the m_minPts-th nearest
    QVector<double> dists;
    dists.reserve(neighbors.size());
    for (int ni : neighbors)
        dists.append(euclideanDist(data[idx], data[ni]));
    std::sort(dists.begin(), dists.end());
    return dists[qMin(m_minPts - 2, dists.size() - 1)];
}

/* ---- Update seeds (priority propagation) ---- */

void OPTICS13::updateSeeds(int idx, const QVector<int>& neighbors,
                            const QVector<QVector<double>>& data,
                            QVector<double>& reach, QVector<bool>& processed,
                            QVector<int>& ordering, int& orderPos)
{
    double cDist = coreDist(idx, data, neighbors);
    for (int ni : neighbors) {
        if (processed[ni]) continue;
        double newReach = qMax(cDist, euclideanDist(data[idx], data[ni]));
        if (newReach < reach[ni]) reach[ni] = newReach;
    }

    // Select unprocessed point with smallest reachability
    double minR = 1e30;
    int best = -1;
    int n = data.size();
    for (int i = 0; i < n; ++i) {
        if (!processed[i] && reach[i] < minR) {
            minR = reach[i];
            best = i;
        }
    }
    if (best >= 0) {
        processed[best] = true;
        ordering[orderPos++] = best;
    }
}

/* ---- Extract clusters via steepness evaluation ---- */

QVector<OPTICS13::Cluster> OPTICS13::extractClusters(
    const QVector<OpticsPoint>& ordering) const
{
    QVector<Cluster> clusters;
    int n = ordering.size();
    if (n == 0) return clusters;

    // Steep-down / steep-up detection using xi threshold
    int i = 0;
    while (i < n) {
        // Detect steep-down region
        int sdStart = i;
        while (i + 1 < n &&
               ordering[i + 1].reachability < ordering[i].reachability * (1.0 - m_xi))
            ++i;
        int sdEnd = i;

        if (sdEnd > sdStart) {
            // Find matching steep-up region
            int j = sdEnd + 1;
            while (j < n &&
                   ordering[j].reachability <= ordering[sdEnd].reachability * (1.0 + m_xi))
                ++j;

            int suStart = j;
            while (j + 1 < n &&
                   ordering[j + 1].reachability > ordering[j].reachability * (1.0 + m_xi))
                ++j;
            int suEnd = j;

            if (suEnd > suStart || sdEnd > sdStart) {
                Cluster c;
                c.isSteepDown = true;
                c.isSteepUp = (suEnd > suStart);
                c.startReach = ordering[sdStart].reachability;
                c.endReach = ordering[qMin(suEnd, n - 1)].reachability;

                // Collect points between steep-down start and steep-up end
                int cEnd = qMax(sdEnd, suEnd);
                for (int k = sdStart; k <= cEnd && k < n; ++k)
                    c.pointIndices.append(ordering[k].index);

                if (!c.pointIndices.isEmpty())
                    clusters.append(c);
            }
        }
        ++i;
    }

    // If no clusters detected, treat all as one cluster
    if (clusters.isEmpty()) {
        Cluster all;
        all.isSteepDown = false;
        all.isSteepUp = false;
        for (const auto& p : ordering)
            all.pointIndices.append(p.index);
        if (!all.pointIndices.isEmpty())
            clusters.append(all);
    }
    return clusters;
}

/* ---- Main OPTICS fitting ---- */

OPTICS13::OpticsResult OPTICS13::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    OpticsResult result;
    int n = data.size();
    if (n == 0) return result;

    QVector<double> reach(n, 1e30);
    QVector<bool> processed(n, false);
    QVector<int> ordering(n, -1);
    int orderPos = 0;

    // Process each unvisited point
    for (int i = 0; i < n; ++i) {
        if (processed[i]) continue;
        processed[i] = true;
        ordering[orderPos++] = i;

        auto nbrs = findNeighbors(i, data);
        if (nbrs.size() >= m_minPts - 1) {
            updateSeeds(i, nbrs, data, reach, processed, ordering, orderPos);
        }
    }

    // Build ordered result
    result.ordering.resize(orderPos);
    for (int j = 0; j < orderPos; ++j) {
        int idx = ordering[j];
        result.ordering[j].index = idx;
        result.ordering[j].reachability = reach[idx];
        auto nbrs = findNeighbors(idx, data);
        result.ordering[j].coreDistance = coreDist(idx, data, nbrs);
    }
    result.numProcessed = orderPos;

    // Extract clusters via steepness
    result.clusters = extractClusters(result.ordering);

    double elapsed = timer.elapsed();
    m_stats.numPoints = n;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit orderingDone(n, result.clusters.size(), elapsed);

    return result;
}

/* ---- Reset ---- */

void OPTICS13::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
