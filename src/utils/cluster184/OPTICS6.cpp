/**
 * @file OPTICS6.cpp
 * @brief OPTICS6 实现
 *
 * 实现OPTICS排序聚类：核心距离、可达距离排序、陡升陡降Xi簇提取。
 */

#include "utils/cluster184/OPTICS6.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

OPTICS6::OPTICS6(QObject *parent) : QObject(parent) {}
OPTICS6::~OPTICS6() = default;

/* ---- Configuration ---- */

void OPTICS6::setEpsilon(double eps) { m_epsilon = qMax(0.001, eps); }
void OPTICS6::setMinPoints(int minPts) { m_minPoints = qMax(2, minPts); }
void OPTICS6::setXi(double xi) { m_xi = qBound(0.01, xi, 1.0); }

/* ---- Distance ---- */

double OPTICS6::distance(const QVector<double>& a, const QVector<double>& b) const
{
    double sum = 0.0;
    int d = qMin(a.size(), b.size());
    for (int i = 0; i < d; ++i) {
        double diff = a[i] - b[i];
        sum += diff * diff;
    }
    return qSqrt(sum);
}

/* ---- Core distance ---- */

double OPTICS6::coreDistance(const QVector<QVector<double>>& data,
                             int idx, const QVector<int>& neighbors) const
{
    if (neighbors.size() < m_minPoints) return std::numeric_limits<double>::infinity();
    QVector<double> dists;
    for (int n : neighbors)
        dists.append(distance(data[idx], data[n]));
    std::sort(dists.begin(), dists.end());
    return dists[m_minPoints - 1];
}

/* ---- Find epsilon-neighborhood ---- */

QVector<int> OPTICS6::findNeighbors(const QVector<QVector<double>>& data, int idx) const
{
    QVector<int> nbrs;
    int n = data.size();
    for (int i = 0; i < n; ++i) {
        if (i == idx) continue;
        if (distance(data[idx], data[i]) <= m_epsilon)
            nbrs.append(i);
    }
    return nbrs;
}

/* ---- Update seeds ---- */

void OPTICS6::updateSeeds(QVector<QPair<double, int>>& seeds,
                          const QVector<QVector<double>>& data,
                          int pointIdx,
                          const QVector<bool>& processed,
                          QVector<double>& reachDist) const
{
    auto nbrs = findNeighbors(data, pointIdx);
    double cDist = coreDistance(data, pointIdx, nbrs);

    for (int nbr : nbrs) {
        if (processed[nbr]) continue;
        double newReach = qMax(cDist, distance(data[pointIdx], data[nbr]));
        if (newReach < reachDist[nbr]) {
            reachDist[nbr] = newReach;
            // Update in seed list
            for (auto& s : seeds) {
                if (s.second == nbr) { s.first = newReach; break; }
            }
        }
    }
}

/* ---- Steep-down detection ---- */

QPair<int, int> OPTICS6::findSteepDown(const QVector<double>& reach, int start) const
{
    int n = reach.size();
    int end = start;
    while (end + 1 < n && reach[end + 1] <= reach[end] * (1.0 - m_xi))
        ++end;
    return {start, end};
}

/* ---- Steep-up detection ---- */

QPair<int, int> OPTICS6::findSteepUp(const QVector<double>& reach, int start) const
{
    int n = reach.size();
    int end = start;
    while (end + 1 < n && reach[end + 1] >= reach[end] * (1.0 + m_xi))
        ++end;
    return {start, end};
}

/* ---- Xi cluster extraction ---- */

QVector<QVector<int>> OPTICS6::extractClustersXi(
    const QVector<double>& reachability, const QVector<int>& ordering) const
{
    QVector<QVector<int>> clusters;
    int n = reachability.size();
    if (n < 3) return clusters;

    int i = 0;
    while (i < n - 1) {
        // Look for steep-down followed by steep-up
        auto down = findSteepDown(reachability, i);
        if (down.first < down.second) {
            // Found steep-down, now look for matching steep-up
            int j = down.second + 1;
            while (j < n - 1) {
                auto up = findSteepUp(reachability, j);
                if (up.first < up.second) {
                    // Extract cluster between down area end and up area start
                    QVector<int> cluster;
                    for (int k = down.second; k <= up.first; ++k)
                        cluster.append(ordering[k]);
                    if (cluster.size() >= m_minPoints)
                        clusters.append(cluster);
                    j = up.second + 1;
                    break;
                }
                ++j;
            }
            i = down.second + 1;
        } else {
            ++i;
        }
    }
    return clusters;
}

/* ---- Main fit (Hierholzer-style OPTICS ordering) ---- */

OPTICS6::ClusterResult OPTICS6::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    ClusterResult result;
    int n = data.size();
    if (n == 0) return result;

    QVector<bool> processed(n, false);
    QVector<double> reachDist(n, std::numeric_limits<double>::infinity());
    QVector<double> coreDist(n, std::numeric_limits<double>::infinity());

    // Process each unprocessed point
    for (int i = 0; i < n; ++i) {
        if (processed[i]) continue;

        processed[i] = true;
        result.ordering.append(i);
        result.reachability.append(reachDist[i]);

        auto nbrs = findNeighbors(data, i);
        coreDist[i] = coreDistance(data, i, nbrs);

        if (nbrs.size() >= m_minPoints - 1) {
            // Seed list with (reachability, index)
            QVector<QPair<double, int>> seeds;
            for (int nbr : nbrs) {
                if (processed[nbr]) continue;
                double newReach = qMax(coreDist[i], distance(data[i], data[nbr]));
                reachDist[nbr] = newReach;
                seeds.append({newReach, nbr});
            }

            while (!seeds.isEmpty()) {
                // Pick seed with smallest reachability
                int bestIdx = 0;
                for (int s = 1; s < seeds.size(); ++s) {
                    if (seeds[s].first < seeds[bestIdx].first)
                        bestIdx = s;
                }
                int q = seeds[bestIdx].first < seeds[bestIdx].first ? seeds[bestIdx].second : seeds[bestIdx].second;
                q = seeds.takeAt(bestIdx).second;

                processed[q] = true;
                result.ordering.append(q);
                result.reachability.append(reachDist[q]);

                auto qNbrs = findNeighbors(data, q);
                coreDist[q] = coreDistance(data, q, qNbrs);

                if (qNbrs.size() >= m_minPoints - 1) {
                    for (int nbr : qNbrs) {
                        if (processed[nbr]) continue;
                        double newReach = qMax(coreDist[q], distance(data[q], data[nbr]));
                        if (newReach < reachDist[nbr]) {
                            reachDist[nbr] = newReach;
                            bool found = false;
                            for (auto& s : seeds) {
                                if (s.second == nbr) { s.first = newReach; found = true; break; }
                            }
                            if (!found) seeds.append({newReach, nbr});
                        }
                    }
                }
            }
        }
    }

    result.coreDist = coreDist;

    // Extract clusters via Xi method
    auto clusters = extractClustersXi(result.reachability, result.ordering);
    result.labels.fill(-1, n);
    for (int c = 0; c < clusters.size(); ++c)
        for (int idx : clusters[c])
            result.labels[idx] = c;

    m_stats.totalRuns++;
    m_stats.numPoints = n;
    m_stats.numClusters = clusters.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalRuns;

    emit clusteringCompleted(clusters.size(), timer.elapsed());
    return result;
}

/* ---- Reset ---- */

void OPTICS6::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
