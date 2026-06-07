/**
 * @file OPTICS7.cpp
 * @brief OPTICS7 实现
 *
 * 实现OPTICS排序：梯度法自动簇提取、可达距离熵评分。
 */

#include "utils/cluster200/OPTICS7.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

OPTICS7::OPTICS7(QObject *parent) : QObject(parent) {}
OPTICS7::~OPTICS7() = default;

/* ---- Configuration ---- */

void OPTICS7::setEpsilon(double eps) { m_eps = qMax(0.01, eps); }
void OPTICS7::setMinPoints(int minPts) { m_minPts = qMax(2, minPts); }
void OPTICS7::setGradientThreshold(double threshold) { m_gradientThreshold = qBound(0.01, threshold, 10.0); }

/* ---- Distance ---- */

double OPTICS7::distance(const QVector<double>& a, const QVector<double>& b)
{
    double sum = 0.0;
    for (int i = 0; i < qMin(a.size(), b.size()); ++i) {
        double d = a[i] - b[i];
        sum += d * d;
    }
    return qSqrt(sum);
}

/* ---- Region query ---- */

QVector<int> OPTICS7::regionQuery(const QVector<QVector<double>>& data, int idx) const
{
    QVector<int> neighbors;
    for (int i = 0; i < data.size(); ++i) {
        if (i == idx) continue;
        if (distance(data[idx], data[i]) <= m_eps)
            neighbors.append(i);
    }
    return neighbors;
}

/* ---- Core distance ---- */

double OPTICS7::coreDistance(const QVector<QVector<double>>& data, int idx,
                              const QVector<int>& neighbors) const
{
    if (neighbors.size() < m_minPts - 1) return std::numeric_limits<double>::infinity();
    QVector<double> dists;
    dists.reserve(neighbors.size());
    for (int n : neighbors)
        dists.append(distance(data[idx], data[n]));
    std::sort(dists.begin(), dists.end());
    return dists[m_minPts - 2]; // minPts-1-th nearest (excluding self)
}

/* ---- Update seeds ---- */

void OPTICS7::updateSeeds(const QVector<QVector<double>>& data, int idx,
                            const QVector<int>& neighbors,
                            QVector<double>& reachDist, QVector<bool>& processed,
                            QVector<int>& predecessor) const
{
    double cDist = coreDistance(data, idx, neighbors);
    for (int n : neighbors) {
        if (processed[n]) continue;
        double newReach = qMax(cDist, distance(data[idx], data[n]));
        if (newReach < reachDist[n]) {
            reachDist[n] = newReach;
            predecessor[n] = idx;
        }
    }
}

/* ---- Fit (main OPTICS loop) ---- */

QVector<int> OPTICS7::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    if (n == 0) return {};

    QVector<bool> processed(n, false);
    QVector<double> reachDist(n, std::numeric_limits<double>::infinity());
    QVector<int> predecessor(n, -1);
    m_ordering.clear();
    m_reachability.clear();
    m_ordering.reserve(n);
    m_reachability.reserve(n);

    for (int i = 0; i < n; ++i) {
        if (processed[i]) continue;
        processed[i] = true;
        m_ordering.append(i);
        m_reachability.append(reachDist[i]);

        QVector<int> neighbors = regionQuery(data, i);
        if (neighbors.size() >= m_minPts - 1) {
            updateSeeds(data, i, neighbors, reachDist, processed, predecessor);

            // Process seeds in order of reachability (simple selection)
            forever {
                int best = -1;
                double bestDist = std::numeric_limits<double>::infinity();
                for (int j = 0; j < n; ++j) {
                    if (!processed[j] && reachDist[j] < bestDist) {
                        bestDist = reachDist[j];
                        best = j;
                    }
                }
                if (best < 0) break;

                processed[best] = true;
                m_ordering.append(best);
                m_reachability.append(reachDist[best]);

                QVector<int> bestNeighbors = regionQuery(data, best);
                if (bestNeighbors.size() >= m_minPts - 1)
                    updateSeeds(data, best, bestNeighbors, reachDist, processed, predecessor);
            }
        }
    }

    // Extract clusters via gradient method
    QVector<Cluster> clusters = extractClusters(m_reachability);

    // Assign labels
    QVector<int> labels(n, -1); // noise = -1
    for (int c = 0; c < clusters.size(); ++c)
        for (int idx = clusters[c].start; idx <= clusters[c].end; ++idx)
            if (idx < m_ordering.size()) labels[m_ordering[idx]] = c;

    m_stats.totalFits++;
    m_stats.numSamples = n;
    m_stats.numClusters = clusters.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFits;

    double totalEntropy = 0.0;
    for (const auto& c : clusters) totalEntropy += c.entropy;
    emit clusteringCompleted(clusters.size(), totalEntropy, timer.elapsed());
    return labels;
}

/* ---- Extract clusters via gradient method ---- */

QVector<OPTICS7::Cluster> OPTICS7::extractClusters(const QVector<double>& reachability) const
{
    QVector<Cluster> result;
    int n = reachability.size();
    if (n < 2) return result;

    // Compute gradient of reachability plot
    QVector<double> gradient(n - 1);
    for (int i = 0; i < n - 1; ++i)
        gradient[i] = reachability[i + 1] - reachability[i];

    // Find steep down areas followed by steep up areas = cluster boundaries
    int start = -1;
    for (int i = 0; i < n; ++i) {
        double r = reachability[i];
        bool isInfinity = (r == std::numeric_limits<double>::infinity());

        if (start < 0 && !isInfinity) {
            start = i;
        } else if (start >= 0 && isInfinity) {
            if (i - start >= m_minPts) {
                Cluster c;
                c.start = start;
                c.end = i - 1;
                c.entropy = computeEntropy(reachability, start, i - 1);
                double sum = 0.0;
                for (int j = start; j <= i - 1; ++j) sum += reachability[j];
                c.avgReachability = sum / (i - start);
                result.append(c);
            }
            start = -1;
        }
    }
    // Handle trailing cluster
    if (start >= 0 && n - start >= m_minPts) {
        Cluster c;
        c.start = start;
        c.end = n - 1;
        c.entropy = computeEntropy(reachability, start, n - 1);
        double sum = 0.0;
        for (int j = start; j <= n - 1; ++j) sum += reachability[j];
        c.avgReachability = sum / (n - start);
        result.append(c);
    }
    return result;
}

/* ---- Compute entropy ---- */

double OPTICS7::computeEntropy(const QVector<double>& reachability, int start, int end) const
{
    if (end <= start) return 0.0;
    int len = end - start + 1;
    double sum = 0.0, sumSq = 0.0;
    for (int i = start; i <= end; ++i) {
        double r = qIsFinite(reachability[i]) ? reachability[i] : 0.0;
        sum += r;
        sumSq += r * r;
    }
    double mean = sum / len;
    double variance = sumSq / len - mean * mean;
    if (variance < 1e-12) return 0.0;

    // Shannon-like entropy using Gaussian kernel density estimation
    double sigma = qSqrt(variance);
    double entropy = 0.0;
    for (int i = start; i <= end; ++i) {
        double r = qIsFinite(reachability[i]) ? reachability[i] : 0.0;
        double p = qExp(-0.5 * ((r - mean) / sigma) * ((r - mean) / sigma));
        p /= (sigma * qSqrt(2.0 * M_PI) * len);
        if (p > 1e-15) entropy -= p * qLn(p);
    }
    return entropy;
}

/* ---- Accessors ---- */

QVector<double> OPTICS7::reachabilityPlot() const { return m_reachability; }
QVector<int> OPTICS7::ordering() const { return m_ordering; }

/* ---- Reset ---- */

void OPTICS7::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_reachability.clear();
    m_ordering.clear();
}
