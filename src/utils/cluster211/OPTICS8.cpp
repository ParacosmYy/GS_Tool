/**
 * @file OPTICS8.cpp
 * @brief OPTICS8 实现
 *
 * 实现OPTICS聚类：可达距离图、核心距离、陡降/陡升自动提取。
 */

#include "utils/cluster211/OPTICS8.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

OPTICS8::OPTICS8(QObject *parent) : QObject(parent) {}
OPTICS8::~OPTICS8() = default;

/* ---- Configuration ---- */

void OPTICS8::setParameters(double epsilon, int minPts)
{
    m_epsilon = qMax(1e-6, epsilon);
    m_minPts = qMax(2, minPts);
}

/* ---- Euclidean distance ---- */

double OPTICS8::euclidean(const QVector<double>& a, const QVector<double>& b)
{
    double sum = 0.0;
    int dim = qMin(a.size(), b.size());
    for (int i = 0; i < dim; ++i) {
        double d = a[i] - b[i];
        sum += d * d;
    }
    return qSqrt(sum);
}

/* ---- Epsilon-neighborhood ---- */

QVector<QPair<double, int>> OPTICS8::epsilonNeighbors(
    int pointIdx, const QVector<QVector<double>>& data) const
{
    QVector<QPair<double, int>> neighbors;
    for (int i = 0; i < m_n; ++i) {
        if (i == pointIdx) continue;
        double dist = euclidean(data[pointIdx], data[i]);
        if (dist <= m_epsilon)
            neighbors.append({dist, i});
    }
    std::sort(neighbors.begin(), neighbors.end());
    return neighbors;
}

/* ---- Core distance ---- */

double OPTICS8::coreDistance(int pointIdx,
                             const QVector<QVector<double>>& data) const
{
    auto neighbors = epsilonNeighbors(pointIdx, data);
    if (neighbors.size() < m_minPts - 1)
        return std::numeric_limits<double>::infinity();
    return neighbors[m_minPts - 2].first;
}

/* ---- Update seeds ---- */

void OPTICS8::updateSeeds(int pointIdx,
                          const QVector<QVector<double>>& data,
                          QVector<double>& reachDist,
                          QVector<bool>& processed,
                          QVector<int>& /*predecessors*/,
                          QVector<QPair<double, int>>& seeds) const
{
    double coreDist = coreDistance(pointIdx, data);

    for (int i = 0; i < m_n; ++i) {
        if (i == pointIdx || processed[i]) continue;
        double dist = euclidean(data[pointIdx], data[i]);
        if (dist > m_epsilon) continue;

        double newReach = qMax(coreDist, dist);
        if (newReach < reachDist[i]) {
            reachDist[i] = newReach;
            // Check if already in seeds; if so update, else add
            bool found = false;
            for (auto& s : seeds) {
                if (s.second == i) {
                    s.first = newReach;
                    found = true;
                    break;
                }
            }
            if (!found)
                seeds.append({newReach, i});
        }
    }
}

/* ---- Fit OPTICS ---- */

void OPTICS8::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    m_n = data.size();
    if (m_n == 0) return;

    QVector<bool> processed(m_n, false);
    QVector<double> reachDist(m_n, std::numeric_limits<double>::infinity());
    QVector<int> predecessors(m_n, -1);
    m_ordering.clear();
    m_ordering.reserve(m_n);
    m_coreDist.resize(m_n);

    for (int i = 0; i < m_n; ++i)
        m_coreDist[i] = coreDistance(i, data);

    for (int i = 0; i < m_n; ++i) {
        if (processed[i]) continue;
        processed[i] = true;
        m_ordering.append(i);

        if (m_coreDist[i] < std::numeric_limits<double>::infinity()) {
            QVector<QPair<double, int>> seeds;
            updateSeeds(i, data, reachDist, processed, predecessors, seeds);

            while (!seeds.isEmpty()) {
                std::sort(seeds.begin(), seeds.end());
                int current = seeds.takeFirst().second;

                if (processed[current]) continue;
                processed[current] = true;
                m_ordering.append(current);

                if (m_coreDist[current] < std::numeric_limits<double>::infinity())
                    updateSeeds(current, data, reachDist,
                               processed, predecessors, seeds);
            }
        }
    }

    // Build reachability plot in OPTICS order
    m_reachability.resize(m_n);
    for (int i = 0; i < m_n; ++i)
        m_reachability[i] = reachDist[m_ordering[i]];

    m_stats.numPoints = m_n;
    m_stats.epsilon = m_epsilon;
    m_stats.minPts = m_minPts;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit clusteringCompleted(m_stats.numClusters, m_n, timer.elapsed());
}

/* ---- Steep down/up detection ---- */

bool OPTICS8::isSteepDown(int i, double xi) const
{
    if (i + 1 >= m_reachability.size()) return false;
    double r1 = m_reachability[i];
    double r2 = m_reachability[i + 1];
    if (r1 <= 0.0) return false;
    return (r1 - r2) / r1 >= xi;
}

bool OPTICS8::isSteepUp(int i, double xi) const
{
    if (i + 1 >= m_reachability.size()) return false;
    double r1 = m_reachability[i];
    double r2 = m_reachability[i + 1];
    if (r1 <= 0.0) return false;
    return (r2 - r1) / r1 >= xi;
}

/* ---- Extract clusters via steep down/up walk ---- */

QVector<OPTICS8::Cluster> OPTICS8::extractClusters(double xi) const
{
    QVector<Cluster> result;
    int n = m_reachability.size();
    if (n < 2) return result;

    int i = 0;
    while (i < n - 1) {
        // Find start of steep down area
        if (!isSteepDown(i, xi)) { ++i; continue; }

        int downStart = i;
        while (i < n - 1 && isSteepDown(i, xi)) ++i;
        int downEnd = i;

        // Find corresponding steep up area
        int upStart = -1;
        int upEnd = -1;
        for (int j = downEnd + 1; j < n - 1; ++j) {
            if (isSteepUp(j, xi)) {
                if (upStart < 0) upStart = j;
                upEnd = j + 1;
                if (j + 1 < n - 1 && !isSteepUp(j + 1, xi))
                    break;
            }
        }

        if (upStart >= 0 && upEnd >= 0) {
            Cluster cluster;
            int cStart = downStart;
            int cEnd = upEnd;
            for (int k = cStart; k <= cEnd && k < m_ordering.size(); ++k)
                cluster.pointIndices.append(m_ordering[k]);
            cluster.startReachability = m_reachability[downStart];
            cluster.endReachability = m_reachability[upEnd];
            result.append(cluster);
        }
        ++i;
    }

    const_cast<OPTICS8*>(this)->m_clusters = result;
    const_cast<OPTICS8*>(this)->m_stats.numClusters = result.size();
    return result;
}

/* ---- Getters ---- */

QVector<double> OPTICS8::reachabilityPlot() const { return m_reachability; }
QVector<int> OPTICS8::ordering() const { return m_ordering; }

/* ---- Reset ---- */

void OPTICS8::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_reachability.clear();
    m_coreDist.clear();
    m_ordering.clear();
    m_clusters.clear();
    m_n = 0;
}
