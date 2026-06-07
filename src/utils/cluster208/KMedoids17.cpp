/**
 * @file KMedoids17.cpp
 * @brief KMedoids17 实现
 *
 * 实现K-中心点聚类：CLARA采样策略、BanditPAM线性时间中心点搜索、轮廓系数评估。
 */

#include "utils/cluster208/KMedoids17.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>
#include <cstdlib>

/* ---- Construction / Destruction ---- */

KMedoids17::KMedoids17(QObject *parent) : QObject(parent) {}
KMedoids17::~KMedoids17() = default;

/* ---- Configuration ---- */

void KMedoids17::setNumMedoids(int k) { m_k = qMax(1, k); }
void KMedoids17::setMaxIterations(int maxIter) { m_maxIter = qMax(1, maxIter); }
void KMedoids17::setSampleSize(int sampleSize) { m_sampleSize = qMax(10, sampleSize); }
void KMedoids17::setNumSamples(int numSamples) { m_numSamples = qMax(1, numSamples); }

/* ---- Euclidean distance ---- */

double KMedoids17::euclidean(const QVector<double>& a, const QVector<double>& b)
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

QVector<QVector<double>> KMedoids17::computeDistanceMatrix(
    const QVector<QVector<double>>& data)
{
    int n = data.size();
    QVector<QVector<double>> dist(n);
    for (int i = 0; i < n; ++i) {
        dist[i].resize(n, 0.0);
        for (int j = 0; j < i; ++j) {
            double d = euclidean(data[i], data[j]);
            dist[i][j] = d;
            dist[j][i] = d;
        }
    }
    return dist;
}

/* ---- Assign labels ---- */

QVector<int> KMedoids17::assignLabels(int n, const QVector<int>& meds) const
{
    QVector<int> labels(n, 0);
    for (int i = 0; i < n; ++i) {
        double bestDist = std::numeric_limits<double>::max();
        for (int m = 0; m < meds.size(); ++m) {
            if (meds[m] < n && m_dist[i][meds[m]] < bestDist) {
                bestDist = m_dist[i][meds[m]];
                labels[i] = m;
            }
        }
    }
    return labels;
}

/* ---- Compute cost ---- */

double KMedoids17::computeCost(int n, const QVector<int>& meds) const
{
    double cost = 0.0;
    for (int i = 0; i < n; ++i) {
        double bestDist = std::numeric_limits<double>::max();
        for (int m : meds) {
            if (m < n) bestDist = qMin(bestDist, m_dist[i][m]);
        }
        cost += bestDist;
    }
    return cost;
}

/* ---- Target distances for BanditPAM ---- */

QVector<double> KMedoids17::computeTargetDistances(
    int n, const QVector<int>& meds) const
{
    QVector<double> targets(n, 0.0);
    for (int i = 0; i < n; ++i) {
        double bestDist = std::numeric_limits<double>::max();
        for (int m : meds) {
            if (m < n) bestDist = qMin(bestDist, m_dist[i][m]);
        }
        targets[i] = bestDist;
    }
    return targets;
}

/* ---- Build step (BanditPAM initialization) ---- */

QVector<int> KMedoids17::buildStep(int n) const
{
    QVector<int> meds;
    if (n <= m_k) {
        for (int i = 0; i < n; ++i) meds.append(i);
        return meds;
    }

    // First medoid: point with smallest total distance
    double bestTotal = std::numeric_limits<double>::max();
    int first = 0;
    for (int i = 0; i < n; ++i) {
        double total = 0.0;
        for (int j = 0; j < n; ++j) total += m_dist[i][j];
        if (total < bestTotal) { bestTotal = total; first = i; }
    }
    meds.append(first);

    // Greedy BanditPAM: add medoid that reduces cost most
    for (int k = 1; k < m_k; ++k) {
        auto targets = computeTargetDistances(n, meds);
        double bestGain = -1.0;
        int bestIdx = 0;

        for (int i = 0; i < n; ++i) {
            if (meds.contains(i)) continue;
            double gain = 0.0;
            for (int j = 0; j < n; ++j) {
                double diff = targets[j] - m_dist[j][i];
                if (diff > 0) gain += diff;
            }
            if (gain > bestGain) { bestGain = gain; bestIdx = i; }
        }
        meds.append(bestIdx);
    }
    return meds;
}

/* ---- Swap step ---- */

bool KMedoids17::swapStep(int n, QVector<int>& meds) const
{
    auto targets = computeTargetDistances(n, meds);
    double bestImprovement = 0.0;
    int swapMedoid = -1, swapCandidate = -1;

    for (int m = 0; m < meds.size(); ++m) {
        for (int i = 0; i < n; ++i) {
            if (meds.contains(i)) continue;
            double delta = 0.0;
            for (int j = 0; j < n; ++j) {
                double currentDist = targets[j];
                double newDist = m_dist[j][i];
                // Second closest medoid distance for j
                double secondBest = std::numeric_limits<double>::max();
                for (int mm = 0; mm < meds.size(); ++mm) {
                    if (mm != m && meds[mm] < n)
                        secondBest = qMin(secondBest, m_dist[j][meds[mm]]);
                }
                double withSwap = qMin(newDist, secondBest);
                delta += withSwap - currentDist;
            }
            if (delta < -bestImprovement) {
                bestImprovement = -delta;
                swapMedoid = m;
                swapCandidate = i;
            }
        }
    }

    if (swapMedoid >= 0) {
        meds[swapMedoid] = swapCandidate;
        return true;
    }
    return false;
}

/* ---- Fit (full BanditPAM) ---- */

void KMedoids17::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();
    int n = data.size();
    if (n == 0) return;

    m_dist = computeDistanceMatrix(data);
    m_medoids = buildStep(n);

    for (int iter = 0; iter < m_maxIter; ++iter) {
        if (!swapStep(n, m_medoids)) break;
    }

    m_labels = assignLabels(n, m_medoids);

    m_stats.numMedoids = m_k;
    m_stats.numSamples = n;
    m_stats.numDimensions = (n > 0) ? data[0].size() : 0;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit fittingCompleted(m_k, computeCost(n, m_medoids), timer.elapsed());
}

/* ---- Fit CLARA ---- */

void KMedoids17::fitCLARA(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();
    int n = data.size();
    if (n == 0) return;

    auto fullDist = computeDistanceMatrix(data);
    QVector<int> bestMeds;
    double bestCost = std::numeric_limits<double>::max();

    for (int s = 0; s < m_numSamples; ++s) {
        // Draw random sample
        int sampleN = qMin(m_sampleSize, n);
        QVector<int> sampleIdx;
        for (int i = 0; i < sampleN; ++i)
            sampleIdx.append(std::rand() % n);

        // Build distance matrix for sample
        m_dist.resize(sampleN);
        for (int i = 0; i < sampleN; ++i) {
            m_dist[i].resize(sampleN);
            for (int j = 0; j < sampleN; ++j)
                m_dist[i][j] = fullDist[sampleIdx[i]][sampleIdx[j]];
        }

        auto meds = buildStep(sampleN);
        for (int iter = 0; iter < m_maxIter; ++iter) {
            if (!swapStep(sampleN, meds)) break;
        }

        // Map back to original indices
        QVector<int> origMeds;
        for (int m : meds) origMeds.append(sampleIdx[m]);

        double cost = 0.0;
        for (int i = 0; i < n; ++i) {
            double bestD = std::numeric_limits<double>::max();
            for (int m : origMeds) bestD = qMin(bestD, fullDist[i][m]);
            cost += bestD;
        }

        if (cost < bestCost) {
            bestCost = cost;
            bestMeds = origMeds;
        }
    }

    m_dist = fullDist;
    m_medoids = bestMeds;
    m_labels = assignLabels(n, m_medoids);

    m_stats.numMedoids = m_k;
    m_stats.numSamples = n;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit fittingCompleted(m_k, bestCost, timer.elapsed());
}

/* ---- Predict ---- */

QVector<int> KMedoids17::predict(const QVector<QVector<double>>& data) const
{
    int n = data.size();
    QVector<int> labels(n, 0);
    for (int i = 0; i < n; ++i) {
        double bestDist = std::numeric_limits<double>::max();
        for (int m = 0; m < m_medoids.size(); ++m) {
            double d = euclidean(data[i], data[m_medoids[m]]);
            if (d < bestDist) { bestDist = d; labels[i] = m; }
        }
    }
    return labels;
}

/* ---- Silhouette score ---- */

double KMedoids17::silhouetteScore(const QVector<QVector<double>>& data) const
{
    int n = data.size();
    if (n <= 1 || m_medoids.isEmpty()) return 0.0;

    auto labels = assignLabels(n, m_medoids);
    auto dist = computeDistanceMatrix(data);
    double total = 0.0;

    for (int i = 0; i < n; ++i) {
        // Intra-cluster distance
        double a = 0.0;
        int countA = 0;
        for (int j = 0; j < n; ++j) {
            if (j != i && labels[j] == labels[i]) { a += dist[i][j]; countA++; }
        }
        a = (countA > 0) ? a / countA : 0.0;

        // Nearest-cluster distance
        double b = std::numeric_limits<double>::max();
        for (int c = 0; c < m_k; ++c) {
            if (c == labels[i]) continue;
            double avgD = 0.0;
            int countB = 0;
            for (int j = 0; j < n; ++j) {
                if (labels[j] == c) { avgD += dist[i][j]; countB++; }
            }
            if (countB > 0) b = qMin(b, avgD / countB);
        }
        if (b == std::numeric_limits<double>::max()) b = 0.0;

        double denom = qMax(a, b);
        total += (denom > 0) ? (b - a) / denom : 0.0;
    }

    m_stats.silhouetteScore = total / n;
    return m_stats.silhouetteScore;
}

/* ---- Getters ---- */

QVector<int> KMedoids17::medoids() const { return m_medoids; }
QVector<int> KMedoids17::labels() const { return m_labels; }

double KMedoids17::totalCost() const
{
    if (m_medoids.isEmpty()) return 0.0;
    return computeCost(m_labels.size(), m_medoids);
}

/* ---- Reset ---- */

void KMedoids17::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_medoids.clear();
    m_labels.clear();
    m_dist.clear();
}
