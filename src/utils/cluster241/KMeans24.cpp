/**
 * @file KMeans24.cpp
 * @brief KMeans24 实现
 *
 * 实现K-means聚类：Elkan三角不等式加速与Hamerly下界减少距离计算。
 */

#include "utils/cluster241/KMeans24.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

KMeans24::KMeans24(QObject *parent) : QObject(parent) {}
KMeans24::~KMeans24() = default;

/* ---- Configuration ---- */

void KMeans24::setNumClusters(int k) { m_k = qMax(1, k); }
void KMeans24::setMaxIterations(int iters) { m_maxIter = qMax(1, iters); }
void KMeans24::setConvergenceThreshold(double tol) { m_tol = qMax(1e-12, tol); }

/* ---- Squared Euclidean distance ---- */

double KMeans24::sqDist(const QVector<double>& a, const QVector<double>& b) const
{
    double sum = 0.0;
    int d = qMin(a.size(), b.size());
    for (int i = 0; i < d; ++i) {
        double diff = a[i] - b[i];
        sum += diff * diff;
    }
    return sum;
}

/* ---- K-means++ initialization ---- */

void KMeans24::initCentroids()
{
    int n = m_data.size();
    if (n == 0) return;
    int d = m_data[0].size();

    m_centroids.resize(m_k);
    for (auto& c : m_centroids) c.resize(d);

    // First centroid: pick the first sample for determinism
    m_centroids[0] = m_data[0];

    QVector<double> minDist(n, std::numeric_limits<double>::max());
    for (int k = 1; k < m_k; ++k) {
        double totalDist = 0.0;
        for (int i = 0; i < n; ++i) {
            double dx = sqDist(m_data[i], m_centroids[k - 1]);
            minDist[i] = qMin(minDist[i], dx);
            totalDist += minDist[i];
        }
        // Pick next center proportional to distance squared
        double threshold = totalDist * (static_cast<double>(k * 7 + 13) / 100.0);
        double cumSum = 0.0;
        int chosen = 0;
        for (int i = 0; i < n; ++i) {
            cumSum += minDist[i];
            if (cumSum >= threshold) { chosen = i; break; }
        }
        m_centroids[k] = m_data[chosen];
    }
}

/* ---- Inter-centroid half-distances ---- */

QVector<QVector<double>> KMeans24::centroidHalfDistances() const
{
    int k = m_centroids.size();
    QVector<QVector<double>> halfDist(k, QVector<double>(k, 0.0));
    for (int i = 0; i < k; ++i)
        for (int j = i + 1; j < k; ++j) {
            double d = qSqrt(sqDist(m_centroids[i], m_centroids[j])) * 0.5;
            halfDist[i][j] = d;
            halfDist[j][i] = d;
        }
    return halfDist;
}

/* ---- Elkan step with Hamerly bound ---- */

bool KMeans24::elkanStep(QVector<double>& upper,
                          QVector<QVector<double>>& lower,
                          QVector<double>& hamerlyBound)
{
    int n = m_data.size();
    int k = m_centroids.size();

    // Compute inter-centroid half distances
    QVector<QVector<double>> halfDist = centroidHalfDistances();

    // For each centroid, find the nearest other centroid
    QVector<double> closestOther(k, std::numeric_limits<double>::max());
    for (int j = 0; j < k; ++j)
        for (int jj = 0; jj < k; ++jj)
            if (jj != j) closestOther[j] = qMin(closestOther[j], 2.0 * halfDist[j][jj]);

    bool changed = false;

    for (int i = 0; i < n; ++i) {
        // Hamerly bound test: skip if upper[i] <= hamerlyBound[i]
        if (upper[i] <= hamerlyBound[i]) {
            m_stats.distancesSkipped += k - 1;
            continue;
        }

        int curAssign = m_labels[i];
        double minDist = std::numeric_limits<double>::max();
        int bestCluster = curAssign;

        for (int j = 0; j < k; ++j) {
            // Elkan triangle inequality lower bound test
            if (j != curAssign && lower[i][j] >= upper[i]) {
                m_stats.distancesSkipped++;
                continue;
            }
            // Elkan: skip if this centroid is not a candidate (halfDist test)
            if (j != curAssign && lower[i][j] >= halfDist[curAssign][j]) {
                m_stats.distancesSkipped++;
                continue;
            }

            // Must compute actual distance
            double d = qSqrt(sqDist(m_data[i], m_centroids[j]));
            m_stats.distancesComputed++;
            lower[i][j] = d;

            if (d < minDist) {
                minDist = d;
                bestCluster = j;
            }
        }

        upper[i] = minDist;
        if (bestCluster != curAssign) {
            m_labels[i] = bestCluster;
            changed = true;
        }

        // Update Hamerly bound: distance to second closest centroid
        double secondBest = std::numeric_limits<double>::max();
        for (int j = 0; j < k; ++j) {
            if (j != m_labels[i]) secondBest = qMin(secondBest, lower[i][j]);
        }
        hamerlyBound[i] = secondBest;
    }

    return changed;
}

/* ---- Update centroids ---- */

void KMeans24::updateCentroids()
{
    int n = m_data.size();
    int k = m_centroids.size();
    if (n == 0) return;
    int d = m_data[0].size();

    QVector<int> counts(k, 0);
    for (auto& c : m_centroids)
        for (int dd = 0; dd < d; ++dd) c[dd] = 0.0;

    for (int i = 0; i < n; ++i) {
        int cl = m_labels[i];
        counts[cl]++;
        for (int dd = 0; dd < d; ++dd)
            m_centroids[cl][dd] += m_data[i][dd];
    }

    for (int j = 0; j < k; ++j) {
        if (counts[j] > 0) {
            for (int dd = 0; dd < d; ++dd)
                m_centroids[j][dd] /= counts[j];
        }
    }
}

/* ---- Fit ---- */

QVector<QVector<double>> KMeans24::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    m_data = data;
    int n = data.size();
    if (n == 0) return m_centroids;
    int d = data[0].size();
    int k = qMin(m_k, n);

    m_k = k;
    m_labels.resize(n, 0);
    m_stats.distancesComputed = 0;
    m_stats.distancesSkipped = 0;

    initCentroids();

    // Initialize Elkan upper bounds and lower bounds
    QVector<double> upper(n, std::numeric_limits<double>::max());
    QVector<QVector<double>> lower(n, QVector<double>(k, 0.0));
    QVector<double> hamerlyBound(n, std::numeric_limits<double>::max());

    // Initial assignment: compute all distances
    for (int i = 0; i < n; ++i) {
        double bestDist = std::numeric_limits<double>::max();
        double secondBest = std::numeric_limits<double>::max();
        int bestCluster = 0;
        for (int j = 0; j < k; ++j) {
            double d = qSqrt(sqDist(m_data[i], m_centroids[j]));
            m_stats.distancesComputed++;
            lower[i][j] = d;
            if (d < bestDist) {
                secondBest = bestDist;
                bestDist = d;
                bestCluster = j;
            } else if (d < secondBest) {
                secondBest = d;
            }
        }
        m_labels[i] = bestCluster;
        upper[i] = bestDist;
        hamerlyBound[i] = secondBest;
    }

    // Iterative refinement with Elkan + Hamerly
    for (int iter = 0; iter < m_maxIter; ++iter) {
        bool changed = elkanStep(upper, lower, hamerlyBound);

        // Save old centroids and update
        auto oldCentroids = m_centroids;
        updateCentroids();

        // Update bounds after centroid movement
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < k; ++j) {
                double move = qSqrt(sqDist(oldCentroids[j], m_centroids[j]));
                lower[i][j] = qMax(0.0, lower[i][j] - move);
            }
            // Upper bound increases by max centroid move
            double maxMove = 0.0;
            for (int j = 0; j < k; ++j)
                maxMove = qMax(maxMove, qSqrt(sqDist(oldCentroids[j], m_centroids[j])));
            upper[i] += maxMove;
            hamerlyBound[i] = qMax(0.0, hamerlyBound[i] - maxMove);
        }

        // Check convergence: max centroid movement
        double maxShift = 0.0;
        for (int j = 0; j < k; ++j)
            maxShift = qMax(maxShift, qSqrt(sqDist(oldCentroids[j], m_centroids[j])));

        emit iterationCompleted(iter, inertia(), m_stats.distancesComputed);
        if (maxShift < m_tol) break;
        if (!changed) break;
    }

    m_stats.numClusters = k;
    m_stats.numSamples = n;
    m_stats.numDimensions = d;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    return m_centroids;
}

/* ---- Accessors ---- */

QVector<int> KMeans24::labels() const { return m_labels; }
QVector<QVector<double>> KMeans24::centroids() const { return m_centroids; }

double KMeans24::inertia() const
{
    double sum = 0.0;
    for (int i = 0; i < m_data.size(); ++i) {
        int cl = m_labels[i];
        if (cl >= 0 && cl < m_centroids.size())
            sum += sqDist(m_data[i], m_centroids[cl]);
    }
    return sum;
}

/* ---- Reset ---- */

void KMeans24::resetStatistics()
{
    m_data.clear(); m_centroids.clear(); m_labels.clear();
    m_stats = Stats{}; m_timeSum = 0.0;
}
