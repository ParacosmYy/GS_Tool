/**
 * @file OPTICS5.cpp
 * @brief OPTICS5 实现
 *
 * 实现OPTICS排序算法：核心距离计算、可达距离传播、
 * 基于优先队列的排序，以及Xi方法层次聚类提取。
 */

#include "utils/cluster162/OPTICS5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

OPTICS5::OPTICS5(QObject* parent)
    : QObject(parent)
{
}

OPTICS5::~OPTICS5() = default;

void OPTICS5::setEpsilon(double eps)
{
    m_epsilon = qMax(1e-10, eps);
}

void OPTICS5::setMinPoints(int minPts)
{
    m_minPts = qMax(2, minPts);
}

void OPTICS5::setXi(double xi)
{
    m_xi = qBound(0.0, xi, 1.0);
}

double OPTICS5::euclidean(const QVector<double>& a, const QVector<double>& b)
{
    double sum = 0.0;
    int dim = qMin(a.size(), b.size());
    for (int i = 0; i < dim; ++i) {
        double d = a[i] - b[i];
        sum += d * d;
    }
    return qSqrt(sum);
}

double OPTICS5::coreDistance(const QVector<int>& neighbors,
                             const QVector<double>& dists) const
{
    if (neighbors.size() < m_minPts) return -1.0;
    /* Sort distances and take the (minPts-1)th as core distance */
    QVector<double> sorted = dists;
    std::sort(sorted.begin(), sorted.end());
    return sorted[m_minPts - 1];
}

void OPTICS5::findNeighbors(const QVector<QVector<double>>& data, int idx,
                            QVector<int>& indices, QVector<double>& dists) const
{
    indices.clear();
    dists.clear();
    for (int j = 0; j < data.size(); ++j) {
        if (j == idx) continue;
        double d = euclidean(data[idx], data[j]);
        if (d <= m_epsilon) {
            indices.append(j);
            dists.append(d);
        }
    }
}

void OPTICS5::updateSeeds(const QVector<QVector<double>>& data, int idx,
                          const QVector<int>& neighbors, const QVector<double>& dists,
                          QVector<double>& reachDist, QVector<bool>& processed,
                          QVector<int>& orderedList, QVector<OrderEntry>& result) const
{
    double cDist = coreDistance(neighbors, dists);
    result.last().coreDistance = cDist;

    if (cDist < 0) return;

    for (int i = 0; i < neighbors.size(); ++i) {
        int nIdx = neighbors[i];
        if (processed[nIdx]) continue;

        double newReach = qMax(cDist, dists[i]);
        if (reachDist[nIdx] < 0 || newReach < reachDist[nIdx]) {
            reachDist[nIdx] = newReach;
        }
    }
}

QVector<OPTICS5::OrderEntry> OPTICS5::run(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    const int n = data.size();
    if (n == 0) return QVector<OrderEntry>();

    QVector<bool> processed(n, false);
    QVector<double> reachDist(n, -1.0);
    QVector<OrderEntry> result;
    result.reserve(n);

    for (int i = 0; i < n; ++i) {
        if (processed[i]) continue;

        processed[i] = true;
        OrderEntry entry{i, -1.0, -1.0};
        result.append(entry);

        QVector<int> neighbors;
        QVector<double> dists;
        findNeighbors(data, i, neighbors, dists);
        updateSeeds(data, i, neighbors, dists, reachDist, processed,
                    neighbors, result);

        /* Seed list: simple linear scan for minimum */
        QVector<int> seeds;
        for (int j = 0; j < n; ++j) {
            if (!processed[j] && reachDist[j] >= 0) {
                seeds.append(j);
            }
        }

        while (true) {
            /* Find unprocessed seed with smallest reachability */
            int bestSeed = -1;
            double bestReach = std::numeric_limits<double>::max();
            for (int s : seeds) {
                if (!processed[s] && reachDist[s] >= 0 && reachDist[s] < bestReach) {
                    bestReach = reachDist[s];
                    bestSeed = s;
                }
            }
            if (bestSeed < 0) break;

            processed[bestSeed] = true;
            result.append({bestSeed, reachDist[bestSeed], -1.0});
            seeds.removeAll(bestSeed);

            findNeighbors(data, bestSeed, neighbors, dists);
            double cDist = coreDistance(neighbors, dists);
            result.last().coreDistance = cDist;

            if (cDist >= 0) {
                for (int i2 = 0; i2 < neighbors.size(); ++i2) {
                    int nIdx = neighbors[i2];
                    if (processed[nIdx]) continue;
                    double newReach = qMax(cDist, dists[i2]);
                    if (reachDist[nIdx] < 0 || newReach < reachDist[nIdx]) {
                        reachDist[nIdx] = newReach;
                        if (!seeds.contains(nIdx)) seeds.append(nIdx);
                    }
                }
            }
        }
    }

    m_stats.totalRuns++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalRuns > 0)
        ? m_timeSum / m_stats.totalRuns : 0.0;

    emit orderingCompleted(result.size());
    return result;
}

QVector<OPTICS5::Cluster> OPTICS5::extractXi(const QVector<OrderEntry>& ordering)
{
    const int n = ordering.size();
    if (n < 3) return QVector<Cluster>();

    QVector<Cluster> clusters;

    /* Compute steep down/up areas using Xi threshold */
    QVector<int> steepDownStart, steepDownEnd;
    QVector<int> steepUpStart, steepUpEnd;

    for (int i = 0; i < n - 1; ++i) {
        double r1 = (ordering[i].reachability < 0) ? 0 : ordering[i].reachability;
        double r2 = (ordering[i + 1].reachability < 0) ? 0 : ordering[i + 1].reachability;

        if (r2 < r1 * (1.0 - m_xi) && r2 > 0) {
            if (steepDownStart.isEmpty() || steepDownEnd.last() != i - 1) {
                steepDownStart.append(i);
            }
            steepDownEnd.append(i);
        }
        if (r2 > r1 * (1.0 + m_xi) && r1 > 0) {
            if (steepUpStart.isEmpty() || steepUpEnd.last() != i - 1) {
                steepUpStart.append(i);
            }
            steepUpEnd.append(i);
        }
    }

    /* Find matching steep-down/steep-up pairs */
    int maxDIdx = qMin(steepDownStart.size(), 5);
    int maxUIdx = qMin(steepUpStart.size(), 5);
    for (int di = 0; di < maxDIdx; ++di) {
        for (int ui = 0; ui < maxUIdx; ++ui) {
            int ds = steepDownStart[di];
            int ue = steepUpEnd[ui];
            if (ue <= ds || ue >= n) continue;

            /* Check: max reachability between end-down and start-up */
            double maxBetween = 0;
            int deEnd = steepDownEnd[di];
            int usStart = steepUpStart[ui];
            for (int k = deEnd; k <= usStart && k < n; ++k) {
                double rv = (ordering[k].reachability < 0) ? 0 : ordering[k].reachability;
                if (rv > maxBetween) maxBetween = rv;
            }

            double dStart = qMax(0.0, ordering[ds].reachability);
            if (dStart < 0 || dStart < 1e-10) continue;

            /* Xi cluster condition */
            if (maxBetween <= dStart * (1.0 - m_xi)) {
                Cluster c;
                c.startOrder = ds;
                c.endOrder = ue;
                for (int k = ds; k <= ue && k < n; ++k) {
                    c.indices.append(ordering[k].index);
                }
                if (c.indices.size() >= m_minPts) {
                    clusters.append(c);
                }
            }
        }
    }

    /* If no clusters found, fall back to simple threshold on reachability */
    if (clusters.isEmpty()) {
        double avgReach = 0;
        int count = 0;
        for (const auto& e : ordering) {
            if (e.reachability > 0) { avgReach += e.reachability; count++; }
        }
        if (count > 0) avgReach /= count;

        Cluster current;
        for (int i = 0; i < n; ++i) {
            double rv = (ordering[i].reachability < 0) ? 0 : ordering[i].reachability;
            if (rv <= avgReach * (1.0 + m_xi)) {
                current.indices.append(ordering[i].index);
            } else {
                if (current.indices.size() >= m_minPts) {
                    clusters.append(current);
                }
                current = Cluster();
            }
        }
        if (current.indices.size() >= m_minPts) clusters.append(current);
    }

    m_stats.lastClusterCount = clusters.size();
    emit xiExtractionCompleted(clusters.size());
    return clusters;
}

void OPTICS5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
