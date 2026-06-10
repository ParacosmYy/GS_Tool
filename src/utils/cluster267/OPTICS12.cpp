/**
 * @file OPTICS12.cpp
 * @brief OPTICS12 实现
 *
 * 实现OPTICS聚类：Xi簇提取与可达距离图分析层次密度聚类。
 */

#include "utils/cluster267/OPTICS12.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

OPTICS12::OPTICS12(QObject *parent)
    : QObject(parent) {}

OPTICS12::~OPTICS12() = default;

/* ---- Configuration ---- */

void OPTICS12::setEpsilon(double eps)
{
    m_epsilon = qMax(1e-6, eps);
}

void OPTICS12::setMinPts(int minPts)
{
    m_minPts = qMax(2, minPts);
}

void OPTICS12::setXi(double xi)
{
    m_xi = qBound(0.01, xi, 0.5);
}

/* ---- Distance ---- */

double OPTICS12::dist(const QVector<QVector<double>>& data, int i, int j) const
{
    double sum = 0.0;
    const auto& a = data[i];
    const auto& b = data[j];
    int dim = qMin(a.size(), b.size());
    for (int d = 0; d < dim; ++d) {
        double diff = a[d] - b[d];
        sum += diff * diff;
    }
    return qSqrt(sum);
}

/* ---- Epsilon-neighborhood ---- */

QVector<int> OPTICS12::neighbors(const QVector<QVector<double>>& data, int idx) const
{
    QVector<int> result;
    for (int i = 0; i < m_n; ++i) {
        if (i == idx) continue;
        if (dist(data, idx, i) <= m_epsilon)
            result.append(i);
    }
    return result;
}

/* ---- Core distance ---- */

double OPTICS12::coreDistance(const QVector<QVector<double>>& data, int idx) const
{
    QVector<double> dists;
    for (int i = 0; i < m_n; ++i) {
        if (i == idx) continue;
        double d = dist(data, idx, i);
        if (d <= m_epsilon) dists.append(d);
    }
    if (dists.size() < m_minPts - 1) return std::numeric_limits<double>::infinity();
    std::sort(dists.begin(), dists.end());
    return dists[m_minPts - 2];
}

/* ---- Main OPTICS ordering ---- */

void OPTICS12::computeOrdering(const QVector<QVector<double>>& data)
{
    m_reachDist.resize(m_n);
    m_coreDist.resize(m_n);
    m_ordering.clear();

    for (int i = 0; i < m_n; ++i) {
        m_reachDist[i] = std::numeric_limits<double>::infinity();
    }

    // Compute core distances
    for (int i = 0; i < m_n; ++i)
        m_coreDist[i] = coreDistance(data, i);

    QVector<bool> processed(m_n, false);

    for (int i = 0; i < m_n; ++i) {
        if (processed[i]) continue;
        processed[i] = true;
        m_ordering.append(i);

        if (std::isinf(m_coreDist[i])) continue;

        // Seed list with priority (reachability distance)
        QVector<QPair<double, int>> seeds;
        auto nbrs = neighbors(data, i);
        for (int nIdx : nbrs) {
            if (processed[nIdx]) continue;
            double newReach = qMax(m_coreDist[i], dist(data, i, nIdx));
            if (newReach < m_reachDist[nIdx]) {
                m_reachDist[nIdx] = newReach;
                seeds.append({newReach, nIdx});
            }
        }

        while (!seeds.isEmpty()) {
            // Pick the seed with smallest reachability
            int best = 0;
            for (int s = 1; s < seeds.size(); ++s) {
                if (seeds[s].first < seeds[best].first) best = s;
            }
            int cur = seeds[best].first;
            int cIdx = seeds[best].second;
            seeds.removeAt(best);

            if (processed[cIdx]) continue;
            processed[cIdx] = true;
            m_ordering.append(cIdx);

            if (std::isinf(m_coreDist[cIdx])) continue;

            auto nbrs2 = neighbors(data, cIdx);
            for (int nIdx : nbrs2) {
                if (processed[nIdx]) continue;
                double newReach = qMax(m_coreDist[cIdx], dist(data, cIdx, nIdx));
                if (newReach < m_reachDist[nIdx]) {
                    m_reachDist[nIdx] = newReach;
                    seeds.append({newReach, nIdx});
                }
            }
        }
    }
}

/* ---- Steep down detection ---- */

bool OPTICS12::isSteepDown(int i, int& end) const
{
    int n = m_ordering.size();
    if (i >= n - 1) { end = i; return false; }
    double threshold = m_reachDist[m_ordering[i]] * (1.0 - m_xi);
    end = i;
    for (int j = i + 1; j < n; ++j) {
        if (m_reachDist[m_ordering[j]] <= threshold ||
            std::isinf(m_reachDist[m_ordering[j]])) {
            end = j;
            threshold = m_reachDist[m_ordering[j]] * (1.0 - m_xi);
        } else {
            break;
        }
    }
    return end > i;
}

/* ---- Steep up detection ---- */

bool OPTICS12::isSteepUp(int i, int& end) const
{
    int n = m_ordering.size();
    if (i >= n - 1) { end = i; return false; }
    double prevReach = m_reachDist[m_ordering[i]];
    end = i;
    for (int j = i + 1; j < n; ++j) {
        double curReach = m_reachDist[m_ordering[j]];
        if (curReach >= prevReach * (1.0 + m_xi)) {
            end = j;
            prevReach = curReach;
        } else {
            break;
        }
    }
    return end > i;
}

/* ---- Xi cluster extraction ---- */

QVector<OPTICS12::Cluster> OPTICS12::extractXiClusters() const
{
    QVector<Cluster> result;
    int n = m_ordering.size();
    if (n < 3) return result;

    int i = 0;
    while (i < n - 1) {
        int downEnd = i;
        if (isSteepDown(i, downEnd)) {
            // Scan for matching steep-up after flat region
            int j = downEnd + 1;
            while (j < n - 1) {
                int upEnd = j;
                if (isSteepUp(j, upEnd)) {
                    // Found cluster: [downStart..upEnd]
                    Cluster cl;
                    for (int k = i; k <= upEnd && k < n; ++k)
                        cl.pointIndices.append(m_ordering[k]);
                    cl.startReach = m_reachDist[m_ordering[i]];
                    cl.endReach = m_reachDist[m_ordering[qMin(upEnd, n - 1)]];
                    if (cl.pointIndices.size() >= m_minPts)
                        result.append(cl);
                    i = upEnd + 1;
                    break;
                }
                ++j;
            }
            if (j >= n - 1) i = downEnd + 1;
        } else {
            ++i;
        }
    }
    return result;
}

/* ---- Full OPTICS fit ---- */

QVector<OPTICS12::Cluster> OPTICS12::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    m_n = data.size();
    if (m_n < 2) return {};

    m_reachDist.clear();
    m_coreDist.clear();
    m_ordering.clear();
    m_clusters.clear();

    computeOrdering(data);
    m_clusters = extractXiClusters();

    double elapsed = timer.elapsed();
    m_stats.numPoints = m_n;
    m_stats.numClusters = m_clusters.size();
    m_stats.epsilon = m_epsilon;
    m_stats.minPts = m_minPts;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit clusteringUpdated(m_stats.numClusters, m_stats.numPoints, elapsed);

    return m_clusters;
}

/* ---- Accessors ---- */

QVector<double> OPTICS12::reachabilityPlot() const { return m_reachDist; }
QVector<int> OPTICS12::ordering() const { return m_ordering; }

/* ---- Reset ---- */

void OPTICS12::resetStatistics()
{
    m_reachDist.clear();
    m_coreDist.clear();
    m_ordering.clear();
    m_clusters.clear();
    m_n = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
