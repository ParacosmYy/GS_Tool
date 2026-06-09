/**
 * @file OPTICS11.cpp
 * @brief OPTICS11 实现
 *
 * 实现OPTICS聚类：Xi簇提取与层次可达距离边界检测。
 */

#include "utils/cluster253/OPTICS11.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

OPTICS11::OPTICS11(QObject *parent)
    : QObject(parent) {}
OPTICS11::~OPTICS11() = default;

/* ---- Configuration ---- */

void OPTICS11::setEpsilon(double eps) { m_epsilon = qMax(0.001, eps); }
void OPTICS11::setMinPoints(int minPts) { m_minPts = qMax(2, minPts); }
void OPTICS11::setXi(double xi) { m_xi = qBound(0.01, xi, 1.0); }

/* ---- Euclidean distance ---- */

double OPTICS11::distance(int i, int j) const
{
    double sum = 0.0;
    for (int d = 0; d < m_dims; ++d) {
        double diff = m_data[i][d] - m_data[j][d];
        sum += diff * diff;
    }
    return qSqrt(sum);
}

/* ---- Find epsilon-neighborhood ---- */

QVector<int> OPTICS11::findNeighbors(int idx) const
{
    QVector<int> neighbors;
    for (int j = 0; j < m_n; ++j) {
        if (j == idx) continue;
        if (distance(idx, j) <= m_epsilon)
            neighbors.append(j);
    }
    return neighbors;
}

/* ---- Compute core distance ---- */

double OPTICS11::computeCoreDist(int idx, const QVector<int>& neighbors) const
{
    if (neighbors.size() < m_minPts - 1)
        return std::numeric_limits<double>::infinity();
    QVector<double> dists;
    dists.reserve(neighbors.size());
    for (int n : neighbors)
        dists.append(distance(idx, n));
    std::sort(dists.begin(), dists.end());
    return dists[m_minPts - 2]; // (minPts-1)-th nearest
}

/* ---- Update seeds with reachability distances ---- */

void OPTICS11::updateSeeds(int idx, QVector<int>& seeds,
                           QVector<double>& seedsRD)
{
    for (int i = 0; i < m_n; ++i) {
        if (m_processed[i]) continue;
        double newRD = qMax(m_coreDist[idx], distance(idx, i));
        // Check if already in seeds
        int pos = -1;
        for (int s = 0; s < seeds.size(); ++s) {
            if (seeds[s] == i) { pos = s; break; }
        }
        if (pos < 0) {
            seeds.append(i);
            seedsRD.append(newRD);
        } else if (newRD < seedsRD[pos]) {
            seedsRD[pos] = newRD;
        }
    }
}

/* ---- Main fit: OPTICS ordering ---- */

QVector<double> OPTICS11::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    m_data = data;
    m_n = data.size();
    if (m_n == 0) return {};
    m_dims = data[0].size();

    m_reachDist.resize(m_n);
    m_coreDist.resize(m_n);
    m_processed.resize(m_n);
    m_order.clear();
    m_clusters.clear();

    for (int i = 0; i < m_n; ++i) {
        m_reachDist[i] = std::numeric_limits<double>::infinity();
        m_processed[i] = false;
    }

    // Process each unprocessed point
    for (int i = 0; i < m_n; ++i) {
        if (m_processed[i]) continue;
        QVector<int> neighbors = findNeighbors(i);
        m_coreDist[i] = computeCoreDist(i, neighbors);
        m_processed[i] = true;
        m_order.append(i);

        if (m_coreDist[i] < std::numeric_limits<double>::infinity()) {
            QVector<int> seeds;
            QVector<double> seedsRD;
            updateSeeds(i, seeds, seedsRD);

            while (!seeds.isEmpty()) {
                // Find seed with smallest reachability distance
                int best = 0;
                for (int s = 1; s < seeds.size(); ++s) {
                    if (seedsRD[s] < seedsRD[best]) best = s;
                }
                int cur = seeds[best];
                double curRD = seedsRD[best];
                seeds.removeAt(best);
                seedsRD.removeAt(best);

                QVector<int> curNeighbors = findNeighbors(cur);
                m_coreDist[cur] = computeCoreDist(cur, curNeighbors);
                m_processed[cur] = true;
                m_reachDist[cur] = curRD;
                m_order.append(cur);

                if (m_coreDist[cur] < std::numeric_limits<double>::infinity()) {
                    updateSeeds(cur, seeds, seedsRD);
                }
            }
        }
    }

    // Extract clusters using Xi method
    extractXiClusters();

    int coreCount = 0;
    for (int i = 0; i < m_n; ++i)
        if (m_coreDist[i] < std::numeric_limits<double>::infinity())
            coreCount++;

    m_stats.numPoints = m_n;
    m_stats.numClusters = m_clusters.size();
    m_stats.numCorePoints = coreCount;
    m_stats.totalOps++;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit clusteringCompleted(m_clusters.size(), coreCount, elapsed);

    QVector<double> result;
    result.reserve(m_order.size());
    for (int idx : m_order)
        result.append(m_reachDist[idx]);
    return result;
}

/* ---- Xi cluster extraction from reachability plot ---- */

void OPTICS11::extractXiClusters()
{
    if (m_order.size() < 3) return;

    // Build reachability plot in OPTICS order
    int sz = m_order.size();
    QVector<double> rd(sz);
    for (int i = 0; i < sz; ++i)
        rd[i] = m_reachDist[m_order[i]];

    double maxRD = 0.0;
    for (int i = 0; i < sz; ++i) {
        if (rd[i] < std::numeric_limits<double>::infinity())
            maxRD = qMax(maxRD, rd[i]);
    }
    if (maxRD <= 0.0) return;

    double threshold = maxRD * m_xi;

    // Find steep down and steep up areas
    for (int i = 1; i < sz - 1; ++i) {
        double prev = (rd[i - 1] < std::numeric_limits<double>::infinity()) ? rd[i - 1] : maxRD;
        double curr = (rd[i] < std::numeric_limits<double>::infinity()) ? rd[i] : maxRD;
        double next = (rd[i + 1] < std::numeric_limits<double>::infinity()) ? rd[i + 1] : maxRD;

        // Steep down followed by steep up = cluster boundary
        if ((prev - curr) > threshold && (next - curr) > threshold) {
            // Scan for matching end boundary
            for (int j = i + 2; j < sz - 1; ++j) {
                double jPrev = (rd[j - 1] < std::numeric_limits<double>::infinity()) ? rd[j - 1] : maxRD;
                double jCurr = (rd[j] < std::numeric_limits<double>::infinity()) ? rd[j] : maxRD;
                double jNext = (rd[j + 1] < std::numeric_limits<double>::infinity()) ? rd[j + 1] : maxRD;

                if ((jCurr - jPrev) > threshold && (jCurr - jNext) > threshold) {
                    Cluster c;
                    for (int k = i; k <= j; ++k)
                        c.pointIndices.append(m_order[k]);
                    c.startXi = rd[i];
                    c.endXi = rd[j];
                    m_clusters.append(c);
                    break;
                }
            }
        }
    }
}

/* ---- Extract clusters accessor ---- */

QVector<OPTICS11::Cluster> OPTICS11::extractClusters() const
{
    return m_clusters;
}

/* ---- Ordering accessor ---- */

QVector<int> OPTICS11::ordering() const { return m_order; }

/* ---- Reset ---- */

void OPTICS11::resetStatistics()
{
    m_data.clear();
    m_reachDist.clear();
    m_coreDist.clear();
    m_order.clear();
    m_processed.clear();
    m_clusters.clear();
    m_n = 0;
    m_dims = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
