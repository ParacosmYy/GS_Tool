/**
 * @file KMedoids21.cpp
 * @brief KMedoids21 实现
 *
 * 实现K-中心点聚类：PAM划分式BUILD-SWAP启发式鲁棒基于样本的聚类。
 */

#include "utils/cluster264/KMedoids21.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

KMedoids21::KMedoids21(QObject *parent)
    : QObject(parent) {}

KMedoids21::~KMedoids21() = default;

/* ---- Configuration ---- */

void KMedoids21::setParameters(int numMedoids, int maxIterations)
{
    m_K = qMax(2, numMedoids);
    m_maxIter = qMax(1, maxIterations);
}

/* ---- Distance computation ---- */

double KMedoids21::euclidean(const QVector<double>& a,
                               const QVector<double>& b) const
{
    double sum = 0.0;
    for (int i = 0; i < a.size() && i < b.size(); ++i) {
        double d = a[i] - b[i];
        sum += d * d;
    }
    return qSqrt(sum);
}

QVector<QVector<double>> KMedoids21::computeDistanceMatrix(
    const QVector<QVector<double>>& data) const
{
    int n = data.size();
    QVector<QVector<double>> dist(n);
    for (int i = 0; i < n; ++i) {
        dist[i].resize(n, 0.0);
        for (int j = i + 1; j < n; ++j) {
            double d = euclidean(data[i], data[j]);
            dist[i][j] = d;
            dist[j][i] = d;
        }
    }
    return dist;
}

/* ---- Build phase: greedy select initial medoids ---- */

void KMedoids21::buildPhase(const QVector<QVector<double>>& data)
{
    if (data.isEmpty()) return;
    m_data = data;
    m_n = data.size();
    m_dim = data[0].size();

    auto dist = computeDistanceMatrix(data);
    m_medoidIdx.clear();

    // Select first medoid: point minimizing total distance to all others
    int first = 0;
    double bestCost = std::numeric_limits<double>::max();
    for (int i = 0; i < m_n; ++i) {
        double cost = 0.0;
        for (int j = 0; j < m_n; ++j) cost += dist[i][j];
        if (cost < bestCost) { bestCost = cost; first = i; }
    }
    m_medoidIdx.append(first);

    // Greedily add remaining medoids
    for (int k = 1; k < m_K; ++k) {
        int best = -1;
        double bestGain = -std::numeric_limits<double>::max();

        for (int c = 0; c < m_n; ++c) {
            if (m_medoidIdx.contains(c)) continue;

            // Compute gain: sum of max(0, current_dist - dist(c,j))
            double gain = 0.0;
            for (int j = 0; j < m_n; ++j) {
                if (m_medoidIdx.contains(j)) continue;
                double curDist = std::numeric_limits<double>::max();
                for (int m : m_medoidIdx)
                    curDist = qMin(curDist, dist[m][j]);
                gain += qMax(0.0, curDist - dist[c][j]);
            }
            if (gain > bestGain) { bestGain = gain; best = c; }
        }
        if (best >= 0) m_medoidIdx.append(best);
    }

    assignToMedoids(dist);
}

/* ---- Assign points to nearest medoid ---- */

void KMedoids21::assignToMedoids(const QVector<QVector<double>>& distMatrix)
{
    m_labels.resize(m_n);
    m_distances.resize(m_n);
    for (int i = 0; i < m_n; ++i) {
        double minDist = std::numeric_limits<double>::max();
        int best = 0;
        for (int k = 0; k < m_medoidIdx.size(); ++k) {
            double d = distMatrix[i][m_medoidIdx[k]];
            if (d < minDist) { minDist = d; best = k; }
        }
        m_labels[i] = best;
        m_distances[i] = minDist;
    }
}

/* ---- Swap phase: iterative improvement ---- */

double KMedoids21::swapCost(const QVector<QVector<double>>& distMatrix,
                              int medIdx, int pointIdx)
{
    // Compute change in total cost if medoid m_medoidIdx[medIdx] is replaced by pointIdx
    int oldMed = m_medoidIdx[medIdx];
    double delta = 0.0;

    for (int i = 0; i < m_n; ++i) {
        if (i == oldMed || i == pointIdx) continue;
        double curDist = m_distances[i];
        double newDist = distMatrix[i][pointIdx];

        // If point i was assigned to the old medoid
        if (m_labels[i] == medIdx) {
            // Find second-best medoid distance
            double secondBest = std::numeric_limits<double>::max();
            for (int k = 0; k < m_medoidIdx.size(); ++k) {
                if (k == medIdx) continue;
                secondBest = qMin(secondBest, distMatrix[i][m_medoidIdx[k]]);
            }
            delta += qMin(newDist, secondBest) - curDist;
        } else {
            // Point assigned to another medoid: might switch to new one
            delta += qMin(0.0, newDist - curDist);
        }
    }
    return delta;
}

void KMedoids21::swapPhase(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    auto dist = computeDistanceMatrix(data);
    int swaps = 0;

    for (int iter = 0; iter < m_maxIter; ++iter) {
        double bestDelta = 0.0;
        int bestMed = -1, bestPoint = -1;

        for (int m = 0; m < m_medoidIdx.size(); ++m) {
            for (int i = 0; i < m_n; ++i) {
                if (m_medoidIdx.contains(i)) continue;
                double delta = swapCost(dist, m, i);
                if (delta < bestDelta) {
                    bestDelta = delta;
                    bestMed = m;
                    bestPoint = i;
                }
            }
        }

        if (bestDelta >= -1e-10) break;  // No improving swap found

        m_medoidIdx[bestMed] = bestPoint;
        assignToMedoids(dist);
        swaps++;
    }

    double elapsed = timer.elapsed();
    m_stats.swapCount = swaps;
    m_stats.totalCost = totalCost();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1ULL, m_stats.totalOps);
    emit clusteringUpdated(m_K, m_stats.totalCost, swaps, elapsed);
}

/* ---- Full PAM fit ---- */

bool KMedoids21::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    if (data.size() < m_K) return false;

    buildPhase(data);

    auto dist = computeDistanceMatrix(data);
    int swaps = 0;

    for (int iter = 0; iter < m_maxIter; ++iter) {
        double bestDelta = 0.0;
        int bestMed = -1, bestPoint = -1;

        for (int m = 0; m < m_medoidIdx.size(); ++m) {
            for (int i = 0; i < m_n; ++i) {
                if (m_medoidIdx.contains(i)) continue;
                double delta = swapCost(dist, m, i);
                if (delta < bestDelta) {
                    bestDelta = delta;
                    bestMed = m;
                    bestPoint = i;
                }
            }
        }

        if (bestDelta >= -1e-10) break;
        m_medoidIdx[bestMed] = bestPoint;
        assignToMedoids(dist);
        swaps++;
    }

    double elapsed = timer.elapsed();
    m_stats.numMedoids = m_K;
    m_stats.numPoints = m_n;
    m_stats.dimension = m_dim;
    m_stats.totalCost = totalCost();
    m_stats.swapCount = swaps;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit clusteringUpdated(m_K, m_stats.totalCost, swaps, elapsed);
    return true;
}

/* ---- Predict ---- */

QVector<int> KMedoids21::predict(const QVector<QVector<double>>& data) const
{
    QVector<int> labels(data.size());
    for (int i = 0; i < data.size(); ++i) {
        double minDist = std::numeric_limits<double>::max();
        int best = 0;
        for (int k = 0; k < m_medoidIdx.size(); ++k) {
            double d = euclidean(data[i], m_data[m_medoidIdx[k]]);
            if (d < minDist) { minDist = d; best = k; }
        }
        labels[i] = best;
    }
    return labels;
}

/* ---- Accessors ---- */

QVector<int> KMedoids21::medoidIndices() const { return m_medoidIdx; }

double KMedoids21::totalCost() const
{
    double cost = 0.0;
    for (double d : m_distances) cost += d;
    return cost;
}

/* ---- Reset ---- */

void KMedoids21::resetStatistics()
{
    m_data.clear();
    m_medoidIdx.clear();
    m_labels.clear();
    m_distances.clear();
    m_n = 0;
    m_dim = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
