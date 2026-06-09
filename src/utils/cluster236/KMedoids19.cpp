/**
 * @file KMedoids19.cpp
 * @brief KMedoids19 实现
 *
 * 实现K-中心点聚类：CLARA采样与PAM构建交换启发式。
 */

#include "utils/cluster236/KMedoids19.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>
#include <random>

/* ---- Construction / Destruction ---- */

KMedoids19::KMedoids19(QObject *parent) : QObject(parent) {}
KMedoids19::~KMedoids19() = default;

/* ---- Configuration ---- */

void KMedoids19::setNumMedoids(int k) { m_k = qMax(2, k); }
void KMedoids19::setSampleSize(int size) { m_sampleSize = qMax(10, size); }
void KMedoids19::setNumSamples(int n) { m_numSamples = qMax(1, n); }
void KMedoids19::setMaxSwapIter(int iter) { m_maxSwapIter = qMax(1, iter); }

/* ---- Distance ---- */

double KMedoids19::distance(const QVector<double>& a, const QVector<double>& b) const
{
    double sum = 0.0;
    int d = qMin(a.size(), b.size());
    for (int i = 0; i < d; ++i) {
        double diff = a[i] - b[i];
        sum += diff * diff;
    }
    return qSqrt(sum);
}

/* ---- Distance matrix ---- */

QVector<QVector<double>> KMedoids19::distanceMatrix(const QVector<QVector<double>>& pts) const
{
    int n = pts.size();
    QVector<QVector<double>> dm(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i)
        for (int j = i + 1; j < n; ++j) {
            double d = distance(pts[i], pts[j]);
            dm[i][j] = d;
            dm[j][i] = d;
        }
    return dm;
}

/* ---- PAM BUILD phase ---- */

QVector<int> KMedoids19::pamBuild(const QVector<QVector<double>>& dist, int k) const
{
    int n = dist.size();
    QVector<int> meds;
    meds.reserve(k);

    // Select first medoid: minimize total distance
    double bestTotal = std::numeric_limits<double>::max();
    int first = 0;
    for (int i = 0; i < n; ++i) {
        double total = 0.0;
        for (int j = 0; j < n; ++j) total += dist[i][j];
        if (total < bestTotal) { bestTotal = total; first = i; }
    }
    meds.append(first);

    // Greedy add remaining medoids
    for (int m = 1; m < k; ++m) {
        double bestGain = -1.0;
        int bestIdx = -1;
        for (int c = 0; c < n; ++c) {
            if (meds.contains(c)) continue;
            double gain = 0.0;
            for (int j = 0; j < n; ++j) {
                double nearest = std::numeric_limits<double>::max();
                for (int mi : meds) nearest = qMin(nearest, dist[mi][j]);
                if (dist[c][j] < nearest) gain += nearest - dist[c][j];
            }
            if (gain > bestGain) { bestGain = gain; bestIdx = c; }
        }
        if (bestIdx >= 0) meds.append(bestIdx);
        else break;
    }
    return meds;
}

/* ---- PAM SWAP phase ---- */

double KMedoids19::pamSwap(const QVector<QVector<double>>& dist, QVector<int>& meds) const
{
    int n = dist.size();
    int k = meds.size();
    double bestCost = 0.0;

    // Compute current assignment costs
    QVector<double> nearestDist(n, std::numeric_limits<double>::max());
    for (int i = 0; i < n; ++i)
        for (int mi : meds)
            nearestDist[i] = qMin(nearestDist[i], dist[i][mi]);
    for (int i = 0; i < n; ++i) bestCost += nearestDist[i];

    for (int iter = 0; iter < m_maxSwapIter; ++iter) {
        double bestSwapDelta = 0.0;
        int swapMedIdx = -1, swapNewIdx = -1;

        for (int mi = 0; mi < k; ++mi) {
            for (int c = 0; c < n; ++c) {
                if (meds.contains(c)) continue;
                // Compute swap cost delta
                double delta = 0.0;
                for (int j = 0; j < n; ++j) {
                    double newDist = dist[j][c];
                    for (int oi = 0; oi < k; ++oi) {
                        if (oi == mi) continue;
                        newDist = qMin(newDist, dist[j][meds[oi]]);
                    }
                    delta += newDist - nearestDist[j];
                }
                if (delta < bestSwapDelta) {
                    bestSwapDelta = delta;
                    swapMedIdx = mi;
                    swapNewIdx = c;
                }
            }
        }

        if (bestSwapDelta >= -1e-12) break; // no improvement
        meds[swapMedIdx] = swapNewIdx;
        bestCost += bestSwapDelta;

        // Recompute nearest distances
        for (int i = 0; i < n; ++i) {
            nearestDist[i] = std::numeric_limits<double>::max();
            for (int mi : meds) nearestDist[i] = qMin(nearestDist[i], dist[i][mi]);
        }
    }
    return bestCost;
}

/* ---- Draw random sample ---- */

QVector<int> KMedoids19::drawSample(int n, int sampleSize) const
{
    int sz = qMin(sampleSize, n);
    QVector<int> indices(n);
    for (int i = 0; i < n; ++i) indices[i] = i;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::shuffle(indices.begin(), indices.end(), gen);
    return indices.mid(0, sz);
}

/* ---- Assign labels ---- */

void KMedoids19::assignLabels()
{
    int n = m_data.size();
    m_labels.resize(n);
    for (int i = 0; i < n; ++i) {
        double minDist = std::numeric_limits<double>::max();
        for (int j = 0; j < m_medoids.size(); ++j) {
            double d = distance(m_data[i], m_data[m_medoids[j]]);
            if (d < minDist) { minDist = d; m_labels[i] = j; }
        }
    }
}

/* ---- Fit ---- */

QVector<int> KMedoids19::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    m_data = data;
    int n = data.size();
    if (n < m_k) return {};

    m_stats.numPoints = n;
    m_stats.numDimensions = data[0].size();

    double bestCost = std::numeric_limits<double>::max();
    QVector<int> bestMedoids;

    for (int s = 0; s < m_numSamples; ++s) {
        QVector<int> sampleIdx = drawSample(n, m_sampleSize);

        // Build subset data
        QVector<QVector<double>> subset;
        for (int idx : sampleIdx) subset.append(m_data[idx]);

        QVector<QVector<double>> dist = distanceMatrix(subset);
        QVector<int> localMeds = pamBuild(dist, m_k);
        double cost = pamSwap(dist, localMeds);

        // Map back to global indices
        QVector<int> globalMeds;
        for (int lm : localMeds) globalMeds.append(sampleIdx[lm]);

        // Evaluate on full dataset
        double fullCost = 0.0;
        for (int i = 0; i < n; ++i) {
            double minD = std::numeric_limits<double>::max();
            for (int gm : globalMeds)
                minD = qMin(minD, distance(m_data[i], m_data[gm]));
            fullCost += minD;
        }

        emit claraSampleCompleted(s, fullCost, timer.elapsed());

        if (fullCost < bestCost) {
            bestCost = fullCost;
            bestMedoids = globalMeds;
        }
    }

    m_medoids = bestMedoids;
    assignLabels();

    m_stats.numMedoids = m_medoids.size();
    m_stats.bestCost = bestCost;
    m_stats.claaraSamples = m_numSamples;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit fitCompleted(m_medoids.size(), bestCost, timer.elapsed());
    return m_medoids;
}

/* ---- Predict ---- */

int KMedoids19::predict(const QVector<double>& point) const
{
    if (m_medoids.isEmpty()) return -1;
    double minDist = std::numeric_limits<double>::max();
    int best = 0;
    for (int j = 0; j < m_medoids.size(); ++j) {
        double d = distance(point, m_data[m_medoids[j]]);
        if (d < minDist) { minDist = d; best = j; }
    }
    return best;
}

/* ---- Accessors ---- */

QVector<int> KMedoids19::medoids() const { return m_medoids; }
QVector<int> KMedoids19::labels() const { return m_labels; }

double KMedoids19::totalCost() const
{
    double cost = 0.0;
    for (int i = 0; i < m_data.size(); ++i) {
        double minD = std::numeric_limits<double>::max();
        for (int j = 0; j < m_medoids.size(); ++j)
            minD = qMin(minD, distance(m_data[i], m_data[m_medoids[j]]));
        cost += minD;
    }
    return cost;
}

/* ---- Reset ---- */

void KMedoids19::resetStatistics()
{
    m_data.clear(); m_medoids.clear(); m_labels.clear();
    m_stats = Stats{}; m_timeSum = 0.0;
}
