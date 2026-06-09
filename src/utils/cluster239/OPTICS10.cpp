/**
 * @file OPTICS10.cpp
 * @brief OPTICS10 实现
 *
 * 实现OPTICS聚类：可达距离图提取与陡下降/上升区域自动簇检测。
 */

#include "utils/cluster239/OPTICS10.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

OPTICS10::OPTICS10(QObject *parent) : QObject(parent) {}
OPTICS10::~OPTICS10() = default;

/* ---- Configuration ---- */

void OPTICS10::setEpsilon(double eps) { m_epsilon = qMax(1e-6, eps); }
void OPTICS10::setMinPoints(int minPts) { m_minPts = qMax(2, minPts); }
void OPTICS10::setSteepThreshold(double t) { m_steepThreshold = qBound(0.0, t, 1.0); }

/* ---- Euclidean distance ---- */

double OPTICS10::euclidean(const QVector<double>& a, const QVector<double>& b) const
{
    double sum = 0.0;
    int d = qMin(a.size(), b.size());
    for (int i = 0; i < d; ++i) {
        double diff = a[i] - b[i];
        sum += diff * diff;
    }
    return qSqrt(sum);
}

/* ---- Find epsilon-neighborhood ---- */

QVector<int> OPTICS10::findNeighbors(int idx) const
{
    QVector<int> result;
    int n = m_data.size();
    for (int i = 0; i < n; ++i) {
        if (i == idx) continue;
        if (euclidean(m_data[idx], m_data[i]) <= m_epsilon)
            result.append(i);
    }
    return result;
}

/* ---- Compute core distance ---- */

double OPTICS10::computeCoreDist(int idx, const QVector<int>& neighbors) const
{
    if (neighbors.size() < m_minPts - 1) return -1.0;

    // Sort neighbors by distance to idx, take the (minPts-1)-th distance
    QVector<double> dists;
    dists.reserve(neighbors.size());
    for (int ni : neighbors)
        dists.append(euclidean(m_data[idx], m_data[ni]));
    std::sort(dists.begin(), dists.end());
    return dists[qMin(m_minPts - 2, dists.size() - 1)];
}

/* ---- Update seeds ---- */

void OPTICS10::updateSeeds(int idx, QVector<double>& reach,
                           QVector<bool>& processed, QVector<int>& seeds)
{
    double cDist = m_coreDist[idx];
    if (cDist < 0) return;

    int n = m_data.size();
    for (int i = 0; i < n; ++i) {
        if (processed[i]) continue;
        double d = euclidean(m_data[idx], m_data[i]);
        if (d > m_epsilon) continue;
        double newReach = qMax(cDist, d);
        if (reach[i] < 0 || newReach < reach[i]) {
            reach[i] = newReach;
            if (!seeds.contains(i)) seeds.append(i);
        }
    }
}

/* ---- Fit: compute OPTICS ordering ---- */

void OPTICS10::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    m_data = data;
    int n = data.size();
    if (n == 0) return;

    m_reachability.resize(n);
    m_coreDist.resize(n);
    m_ordering.clear();
    m_ordering.reserve(n);

    QVector<bool> processed(n, false);
    QVector<int> seeds;

    for (int i = 0; i < n; ++i) {
        m_reachability[i] = -1.0;
        m_coreDist[i] = -1.0;
    }

    for (int i = 0; i < n; ++i) {
        if (processed[i]) continue;
        processed[i] = true;
        m_ordering.append(i);

        QVector<int> neighbors = findNeighbors(i);
        m_coreDist[i] = computeCoreDist(i, neighbors);

        if (m_coreDist[i] >= 0) {
            updateSeeds(i, m_reachability, processed, seeds);
            // Process seeds greedily by smallest reachability
            while (!seeds.isEmpty()) {
                int best = -1;
                double bestR = std::numeric_limits<double>::max();
                for (int s : seeds) {
                    if (!processed[s] && m_reachability[s] >= 0 && m_reachability[s] < bestR) {
                        bestR = m_reachability[s];
                        best = s;
                    }
                }
                if (best < 0) break;
                seeds.removeOne(best);
                processed[best] = true;
                m_ordering.append(best);

                QVector<int> nb = findNeighbors(best);
                m_coreDist[best] = computeCoreDist(best, nb);
                if (m_coreDist[best] >= 0)
                    updateSeeds(best, m_reachability, processed, seeds);
            }
        }
    }

    m_stats.numPoints = n;
    m_stats.numDimensions = (n > 0) ? data[0].size() : 0;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit orderingCompleted(n, timer.elapsed());
}

/* ---- Accessors ---- */

QVector<double> OPTICS10::reachabilityPlot() const
{
    QVector<double> plot;
    plot.reserve(m_ordering.size());
    for (int idx : m_ordering)
        plot.append((m_reachability[idx] >= 0) ? m_reachability[idx] : 0.0);
    return plot;
}

QVector<int> OPTICS10::ordering() const { return m_ordering; }

/* ---- Extract clusters via steep-down/up area detection ---- */

QVector<OPTICS10::ClusterInfo> OPTICS10::extractClusters() const
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> reach = reachabilityPlot();
    int n = reach.size();
    QVector<ClusterInfo> clusters;

    if (n < 3) return clusters;

    // Detect steep-down areas followed by steep-up areas
    QVector<int> steepDownStart, steepDownEnd;
    QVector<int> steepUpStart, steepUpEnd;

    int i = 0;
    while (i < n - 1) {
        // Detect steep-down area
        if (reach[i] > 0 && reach[i + 1] < reach[i] * (1.0 - m_steepThreshold)) {
            int start = i;
            while (i < n - 1 && reach[i] > 0 && reach[i + 1] < reach[i] * (1.0 - m_steepThreshold))
                ++i;
            steepDownStart.append(start);
            steepDownEnd.append(i);
        }
        // Detect steep-up area
        else if (reach[i] > 0 && reach[i + 1] > reach[i] * (1.0 + m_steepThreshold)) {
            int start = i;
            while (i < n - 1 && reach[i] > 0 && reach[i + 1] > reach[i] * (1.0 + m_steepThreshold))
                ++i;
            steepUpStart.append(start);
            steepUpEnd.append(i);
        } else {
            ++i;
        }
    }

    // Match steep-down with steep-up to form clusters
    for (int d = 0; d < steepDownStart.size(); ++d) {
        for (int u = 0; u < steepUpStart.size(); ++u) {
            if (steepUpStart[u] <= steepDownEnd[d]) continue;
            double maxReach = 0.0;
            for (int j = steepDownEnd[d]; j <= steepUpStart[u]; ++j)
                maxReach = qMax(maxReach, reach[j]);

            ClusterInfo ci;
            ci.startReachability = reach[steepDownStart[d]];
            ci.endReachability = reach[steepUpEnd[u]];
            for (int j = steepDownEnd[d]; j <= steepUpStart[u]; ++j)
                if (reach[j] <= maxReach * 0.75)
                    ci.pointIndices.append(m_ordering[j]);
            if (!ci.pointIndices.isEmpty())
                clusters.append(ci);
        }
    }

    m_stats.numClusters = clusters.size();
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit clusterExtracted(clusters.size(), timer.elapsed());
    return clusters;
}

/* ---- Reset ---- */

void OPTICS10::resetStatistics()
{
    m_data.clear(); m_reachability.clear(); m_coreDist.clear(); m_ordering.clear();
    m_stats = Stats{}; m_timeSum = 0.0;
}
