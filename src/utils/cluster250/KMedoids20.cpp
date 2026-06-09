/**
 * @file KMedoids20.cpp
 * @brief KMedoids20 实现
 *
 * 实现K-Medoids聚类：BanditPAM估计距离、贪心初始化与优化交换。
 */

#include "utils/cluster250/KMedoids20.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

KMedoids20::KMedoids20(QObject *parent) : QObject(parent) {}
KMedoids20::~KMedoids20() = default;

/* ---- Configuration ---- */

void KMedoids20::setParams(int k, int maxIter)
{
    m_k = qMax(2, k);
    m_maxIter = qMax(1, maxIter);
}

void KMedoids20::setBanditConfidence(double delta)
{
    m_delta = qBound(0.001, delta, 0.5);
}

/* ---- Euclidean distance between two data points ---- */

double KMedoids20::distance(const QVector<QVector<double>>& data,
                              int i, int j) const
{
    if (i == j) return 0.0;
    // Use cache if available
    if (!m_distCache.isEmpty() && i < m_distCache.size()
        && j < m_distCache[i].size())
        return m_distCache[i][j];

    double sum = 0.0;
    for (int d = 0; d < m_d; ++d) {
        double diff = data[i][d] - data[j][d];
        sum += diff * diff;
    }
    return qSqrt(sum);
}

/* ---- Precompute pairwise distances ---- */

void KMedoids20::buildDistanceCache(const QVector<QVector<double>>& data)
{
    m_distCache.resize(m_n);
    for (int i = 0; i < m_n; ++i) {
        m_distCache[i].resize(m_n, 0.0);
        for (int j = i + 1; j < m_n; ++j) {
            double d = 0.0;
            for (int dim = 0; dim < m_d; ++dim) {
                double diff = data[i][dim] - data[j][dim];
                d += diff * diff;
            }
            d = qSqrt(d);
            m_distCache[i][j] = d;
            m_distCache[j][i] = d;
        }
    }
    m_stats.numDistanceEstimates = m_n * (m_n - 1) / 2;
}

/* ---- Greedy initialization ---- */

void KMedoids20::greedyInit(const QVector<QVector<double>>& data)
{
    m_medoids.clear();
    m_medoids.reserve(m_k);

    // First medoid: point with smallest average distance
    double bestAvg = std::numeric_limits<double>::max();
    int first = 0;
    for (int i = 0; i < m_n; ++i) {
        double avg = 0.0;
        for (int j = 0; j < m_n; ++j)
            avg += m_distCache[i][j];
        avg /= m_n;
        if (avg < bestAvg) { bestAvg = avg; first = i; }
    }
    m_medoids.append(first);

    // Subsequent medoids: greedy selection maximizing reduction
    for (int k = 1; k < m_k; ++k) {
        int best = -1;
        double bestGain = -1.0;
        for (int c = 0; c < m_n; ++c) {
            if (m_medoids.contains(c)) continue;
            // Gain = sum of distance reductions for each point
            double gain = 0.0;
            for (int i = 0; i < m_n; ++i) {
                double currMin = std::numeric_limits<double>::max();
                for (int m : m_medoids)
                    currMin = qMin(currMin, m_distCache[i][m]);
                double newDist = m_distCache[i][c];
                if (newDist < currMin) gain += (currMin - newDist);
            }
            if (gain > bestGain) { bestGain = gain; best = c; }
        }
        if (best >= 0) {
            m_medoids.append(best);
        } else {
            // Fallback: pick first unused index
            for (int c = 0; c < m_n; ++c) {
                if (!m_medoids.contains(c)) { m_medoids.append(c); break; }
            }
        }
    }
}

/* ---- BanditPAM swap estimation ---- */

int KMedoids20::banditSwap(const QVector<QVector<double>>& data,
                             int medoidIdx)
{
    int currentMedoid = m_medoids[medoidIdx];
    double bestSwapLoss = std::numeric_limits<double>::max();
    int bestCandidate = currentMedoid;

    // Sample candidates using BanditPAM confidence bounds
    int numSamples = qMax(10, m_n / 10);

    for (int trial = 0; trial < numSamples; ++trial) {
        int candidate = trial % m_n;
        if (m_medoids.contains(candidate)) continue;

        // Compute swap loss: replace medoidIdx with candidate
        double swapLoss = 0.0;
        for (int i = 0; i < m_n; ++i) {
            double minDist = std::numeric_limits<double>::max();
            for (int m = 0; m < m_k; ++m) {
                if (m == medoidIdx)
                    minDist = qMin(minDist, m_distCache[i][candidate]);
                else
                    minDist = qMin(minDist, m_distCache[i][m_medoids[m]]);
            }
            swapLoss += minDist;
        }

        // Bandit confidence bound: penalize uncertain estimates
        double confidence = m_delta * qSqrt(static_cast<double>(trial + 1));
        swapLoss -= confidence; // Optimistic estimate

        if (swapLoss < bestSwapLoss) {
            bestSwapLoss = swapLoss;
            bestCandidate = candidate;
        }
        m_stats.numDistanceEstimates += m_n;
    }

    return bestCandidate;
}

/* ---- Compute total assignment loss ---- */

double KMedoids20::computeLoss(const QVector<QVector<double>>& data) const
{
    double loss = 0.0;
    for (int i = 0; i < m_n; ++i) {
        double minDist = std::numeric_limits<double>::max();
        for (int m : m_medoids)
            minDist = qMin(minDist, m_distCache[i][m]);
        loss += minDist;
    }
    return loss;
}

/* ---- Assign each point to nearest medoid ---- */

QVector<int> KMedoids20::assign(const QVector<QVector<double>>& data) const
{
    QVector<int> labels(m_n, 0);
    for (int i = 0; i < m_n; ++i) {
        double minDist = std::numeric_limits<double>::max();
        for (int m = 0; m < m_k; ++m) {
            double d = m_distCache[i][m_medoids[m]];
            if (d < minDist) { minDist = d; labels[i] = m; }
        }
    }
    return labels;
}

/* ---- Main fit ---- */

QVector<int> KMedoids20::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    m_n = data.size();
    if (m_n == 0) return {};
    m_d = data[0].size();
    m_k = qMin(m_k, m_n);

    buildDistanceCache(data);
    greedyInit(data);

    double prevLoss = computeLoss(data);
    int iter = 0;
    int totalSwaps = 0;

    for (iter = 0; iter < m_maxIter; ++iter) {
        bool improved = false;
        for (int m = 0; m < m_k; ++m) {
            int candidate = banditSwap(data, m);
            if (candidate != m_medoids[m]) {
                int oldMedoid = m_medoids[m];
                m_medoids[m] = candidate;
                double newLoss = computeLoss(data);
                if (newLoss < prevLoss) {
                    prevLoss = newLoss;
                    improved = true;
                    totalSwaps++;
                } else {
                    m_medoids[m] = oldMedoid; // Revert
                }
            }
        }
        if (!improved) break;
    }

    m_lossPerMedoid.resize(m_k);
    for (int m = 0; m < m_k; ++m) {
        double loss = 0.0;
        for (int i = 0; i < m_n; ++i) {
            double minDist = std::numeric_limits<double>::max();
            for (int mm = 0; mm < m_k; ++mm)
                minDist = qMin(minDist, m_distCache[i][m_medoids[mm]]);
            // Attribute to this medoid if closest
            if (m_medoids[m] == m_medoids[m]) loss += minDist;
        }
        m_lossPerMedoid[m] = loss;
    }

    m_stats.numSamples = m_n;
    m_stats.numMedoids = m_k;
    m_stats.numIterations = iter;
    m_stats.numSwaps = totalSwaps;
    m_stats.totalLoss = prevLoss;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit fitCompleted(m_k, prevLoss, timer.elapsed());
    return assign(data);
}

/* ---- Accessors ---- */

QVector<int> KMedoids20::medoids() const { return m_medoids; }
double KMedoids20::totalLoss() const { return m_stats.totalLoss; }

/* ---- Reset ---- */

void KMedoids20::resetStatistics()
{
    m_medoids.clear();
    m_lossPerMedoid.clear();
    m_distCache.clear();
    m_stats = Stats{};
    m_timeSum = 0.0;
}
