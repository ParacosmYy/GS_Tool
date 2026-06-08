/**
 * @file KMedoids18.cpp
 * @brief KMedoids18 实现
 *
 * 实现K-Medoids：Voronoi交替迭代、Bandit臂抽取加速中心点评估。
 */

#include "utils/cluster222/KMedoids18.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>
#include <random>

/* ---- Construction / Destruction ---- */

KMedoids18::KMedoids18(QObject *parent) : QObject(parent) {}
KMedoids18::~KMedoids18() = default;

/* ---- Configuration ---- */

void KMedoids18::setParameters(int numMedoids, int maxIterations, int banditSamples)
{
    m_numMedoids = qMax(2, numMedoids);
    m_maxIterations = qMax(10, maxIterations);
    m_banditSamples = qMax(1, banditSamples);
}

/* ---- Distance ---- */

double KMedoids18::distance(const QVector<double>& a, const QVector<double>& b) const
{
    double sum = 0.0;
    int dim = qMin(a.size(), b.size());
    for (int i = 0; i < dim; ++i) {
        double d = a[i] - b[i];
        sum += d * d;
    }
    return qSqrt(sum);
}

/* ---- Build initial medoids (greedy BUILD) ---- */

QVector<int> KMedoids18::buildInitialMedoids(const QVector<QVector<double>>& data) const
{
    int n = data.size();
    int k = qMin(m_numMedoids, n);
    QVector<int> medoids;
    QSet<int> selected;

    // First medoid: point with smallest total distance
    double bestTotal = std::numeric_limits<double>::max();
    int firstMedoid = 0;
    for (int i = 0; i < n; ++i) {
        double total = 0.0;
        for (int j = 0; j < n; ++j)
            total += distance(data[i], data[j]);
        if (total < bestTotal) { bestTotal = total; firstMedoid = i; }
    }
    medoids.append(firstMedoid);
    selected.insert(firstMedoid);

    // Remaining medoids: greedy selection maximizing cost reduction
    for (int m = 1; m < k; ++m) {
        double bestGain = -1.0;
        int bestIdx = 0;
        for (int i = 0; i < n; ++i) {
            if (selected.contains(i)) continue;
            // Compute gain: how much total cost decreases
            double gain = 0.0;
            for (int j = 0; j < n; ++j) {
                double currentDist = std::numeric_limits<double>::max();
                for (int med : medoids)
                    currentDist = qMin(currentDist, distance(data[med], data[j]));
                double newDist = distance(data[i], data[j]);
                if (newDist < currentDist) gain += currentDist - newDist;
            }
            if (gain > bestGain) { bestGain = gain; bestIdx = i; }
        }
        medoids.append(bestIdx);
        selected.insert(bestIdx);
    }
    return medoids;
}

/* ---- Build Voronoi assignments ---- */

QVector<int> KMedoids18::buildVoronoi(const QVector<int>& medoids,
                                         const QVector<QVector<double>>& data) const
{
    int n = data.size();
    QVector<int> assignments(n, 0);
    for (int i = 0; i < n; ++i) {
        double bestDist = std::numeric_limits<double>::max();
        for (int m = 0; m < medoids.size(); ++m) {
            double d = distance(data[i], data[medoids[m]]);
            if (d < bestDist) { bestDist = d; assignments[i] = m; }
        }
    }
    return assignments;
}

/* ---- Total cost ---- */

double KMedoids18::computeTotalCost(const QVector<int>& medoids,
                                      const QVector<QVector<double>>& data) const
{
    double total = 0.0;
    for (int i = 0; i < data.size(); ++i) {
        double bestDist = std::numeric_limits<double>::max();
        for (int med : medoids)
            bestDist = qMin(bestDist, distance(data[med], data[i]));
        total += bestDist;
    }
    return total;
}

/* ---- Bandit-based candidate selection ---- */

int KMedoids18::banditSelectCandidate(int medoidIdx,
                                         const QVector<int>& medoids,
                                         const QVector<QVector<double>>& data) const
{
    int n = data.size();
    int numSamples = qMin(m_banditSamples, n);

    // UCB1-style arm pulling: sample candidates, estimate swap improvement
    static thread_local std::mt19937 rng(12345);
    QVector<int> candidates;
    QVector<double> upperBounds(n, 0.0);
    QVector<int> pullCounts(n, 0);
    QVector<double> pullRewards(n, 0.0);

    // Initial pull for all non-medoid points
    for (int i = 0; i < n; ++i) {
        if (medoids.contains(i)) continue;
        double reward = 0.0;
        int samples = qMin(5, n);
        for (int s = 0; s < samples; ++s) {
            int j = rng() % n;
            double oldDist = std::numeric_limits<double>::max();
            for (int med : medoids)
                oldDist = qMin(oldDist, distance(data[med], data[j]));
            QVector<int> trialMedoids = medoids;
            trialMedoids[medoidIdx] = i;
            double newDist = std::numeric_limits<double>::max();
            for (int med : trialMedoids)
                newDist = qMin(newDist, distance(data[med], data[j]));
            reward += (oldDist - newDist);
        }
        pullCounts[i] = samples;
        pullRewards[i] = reward;
    }

    // Additional bandit rounds
    for (int round = 0; round < numSamples; ++round) {
        // Select arm with highest UCB
        int bestArm = -1;
        double bestUCB = -std::numeric_limits<double>::max();
        for (int i = 0; i < n; ++i) {
            if (medoids.contains(i) || pullCounts[i] == 0) continue;
            double avg = pullRewards[i] / pullCounts[i];
            double ucb = avg + qSqrt(2.0 * qLn(round + 2) / pullCounts[i]);
            if (ucb > bestUCB) { bestUCB = ucb; bestArm = i; }
        }
        if (bestArm < 0) break;

        // Pull arm: evaluate swap with random sample point
        int j = rng() % n;
        double oldDist = std::numeric_limits<double>::max();
        for (int med : medoids)
            oldDist = qMin(oldDist, distance(data[med], data[j]));
        QVector<int> trialMedoids = medoids;
        trialMedoids[medoidIdx] = bestArm;
        double newDist = std::numeric_limits<double>::max();
        for (int med : trialMedoids)
            newDist = qMin(newDist, distance(data[med], data[j]));
        pullRewards[bestArm] += (oldDist - newDist);
        pullCounts[bestArm]++;
    }

    // Select candidate with best average reward
    int bestCandidate = medoids[medoidIdx];
    double bestAvg = 0.0;
    for (int i = 0; i < n; ++i) {
        if (medoids.contains(i) || pullCounts[i] == 0) continue;
        double avg = pullRewards[i] / pullCounts[i];
        if (avg > bestAvg) { bestAvg = avg; bestCandidate = i; }
    }
    return bestCandidate;
}

/* ---- Fit ---- */

KMedoids18::MedoidResult KMedoids18::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    if (data.isEmpty()) return MedoidResult();
    m_data = data;
    int n = data.size();
    int k = qMin(m_numMedoids, n);

    m_stats.numPoints = n;
    m_stats.numMedoids = k;
    m_stats.dim = data[0].size();

    // BUILD phase
    QVector<int> medoids = buildInitialMedoids(data);
    double currentCost = computeTotalCost(medoids, data);

    MedoidResult result;
    int iter = 0;

    // SWAP phase with Voronoi + Bandit
    for (iter = 0; iter < m_maxIterations; ++iter) {
        bool improved = false;

        // Voronoi assignment
        QVector<int> assignments = buildVoronoi(medoids, data);

        // For each medoid, use bandit to find better candidate
        for (int m = 0; m < k; ++m) {
            int candidate = banditSelectCandidate(m, medoids, data);
            if (candidate != medoids[m]) {
                QVector<int> trialMedoids = medoids;
                trialMedoids[m] = candidate;
                double trialCost = computeTotalCost(trialMedoids, data);
                if (trialCost < currentCost) {
                    medoids[m] = candidate;
                    currentCost = trialCost;
                    improved = true;
                }
            }
        }

        if (!improved) break;
    }

    result.medoidIndices = medoids;
    result.assignments = buildVoronoi(medoids, data);
    result.totalCost = currentCost;
    result.iterations = iter + 1;

    // Compute per-medoid costs
    result.medoidCosts.resize(k);
    for (int m = 0; m < k; ++m) {
        double cost = 0.0;
        for (int i = 0; i < n; ++i) {
            if (result.assignments[i] == m)
                cost += distance(data[medoids[m]], data[i]);
        }
        result.medoidCosts[m] = cost;
    }

    m_medoidIndices = medoids;
    m_stats.totalIterations += iter + 1;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit clusteringCompleted(k, iter + 1, currentCost, timer.elapsed());
    return result;
}

/* ---- Predict ---- */

QVector<int> KMedoids18::predict(const QVector<QVector<double>>& data) const
{
    return buildVoronoi(m_medoidIndices, data);
}

/* ---- Accessor ---- */

QVector<int> KMedoids18::medoidIndices() const { return m_medoidIndices; }

/* ---- Reset ---- */

void KMedoids18::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_medoidIndices.clear();
    m_data.clear();
}
