/**
 * @file KMedoids22.cpp
 * @brief KMedoids22 实现
 *
 * 实现K-中心点聚类：CLARA采样与多起点PAM的大规模可扩展中心点聚类。
 */

#include "utils/cluster278/KMedoids22.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

KMedoids22::KMedoids22(QObject *parent)
    : QObject(parent) {}

KMedoids22::~KMedoids22() = default;

/* ---- Configuration ---- */

void KMedoids22::setK(int k) { m_k = qBound(2, k, 100); }
void KMedoids22::setMaxIterations(int iter) { m_maxIter = qBound(10, iter, 500); }
void KMedoids22::setClaraSamples(int samples) { m_claraSamples = qBound(1, samples, 50); }
void KMedoids22::setSampleFraction(double frac) { m_sampleFrac = qBound(0.05, frac, 1.0); }
void KMedoids22::setMultiStarts(int starts) { m_multiStarts = qBound(1, starts, 20); }

/* ---- Euclidean distance ---- */

double KMedoids22::euclidean(const QVector<double>& a, const QVector<double>& b) const
{
    double sum = 0.0;
    int dim = qMin(a.size(), b.size());
    for (int i = 0; i < dim; ++i) {
        double d = a[i] - b[i];
        sum += d * d;
    }
    return qSqrt(sum);
}

/* ---- Build pairwise distance matrix ---- */

void KMedoids22::buildDistMatrix(const QVector<QVector<double>>& data)
{
    int n = data.size();
    m_dist = QVector<QVector<double>>(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i)
        for (int j = i + 1; j < n; ++j) {
            double d = euclidean(data[i], data[j]);
            m_dist[i][j] = d;
            m_dist[j][i] = d;
        }
}

/* ---- Total cost for given medoid set ---- */

double KMedoids22::totalCost(const QVector<int>& medoids,
                              const QVector<int>& pointIdx) const
{
    double cost = 0.0;
    for (int p : pointIdx) {
        double minD = 1e30;
        for (int m : medoids) {
            if (m < m_dist.size() && p < m_dist[m].size())
                minD = qMin(minD, m_dist[m][p]);
        }
        cost += minD;
    }
    return cost;
}

/* ---- Assign labels to nearest medoid ---- */

QVector<int> KMedoids22::assignLabels(const QVector<int>& medoids,
                                       const QVector<int>& pointIdx) const
{
    QVector<int> labels(pointIdx.size(), 0);
    for (int i = 0; i < pointIdx.size(); ++i) {
        int p = pointIdx[i];
        double minD = 1e30;
        int best = 0;
        for (int j = 0; j < medoids.size(); ++j) {
            int m = medoids[j];
            if (m < m_dist.size() && p < m_dist[m].size()) {
                double d = m_dist[m][p];
                if (d < minD) { minD = d; best = j; }
            }
        }
        labels[i] = best;
    }
    return labels;
}

/* ---- Single PAM run on index subset ---- */

KMedoids22::MedoidResult KMedoids22::runPAM(const QVector<int>& pointIdx, int k) const
{
    MedoidResult result;
    int n = pointIdx.size();
    if (n < k) return result;

    // Initialize: select k random medoids from pointIdx
    QVector<int> medoids(k);
    QVector<bool> used(n, false);
    for (int i = 0; i < k; ++i) {
        int r;
        do { r = static_cast<int>(QRandomGenerator::global()->bounded(n)); } while (used[r]);
        used[r] = true;
        medoids[i] = pointIdx[r];
    }

    double bestCost = totalCost(medoids, pointIdx);
    bool swapped = true;
    int iter = 0;

    // PAM SWAP: try swapping each medoid with each non-medoid
    while (swapped && iter < m_maxIter) {
        swapped = false;
        iter++;

        for (int i = 0; i < k; ++i) {
            int oldMed = medoids[i];
            double bestSwapCost = bestCost;
            int bestSwap = -1;

            for (int j = 0; j < n; ++j) {
                int cand = pointIdx[j];
                if (cand == oldMed) continue;
                // Check cand not already a medoid
                bool isMed = false;
                for (int m = 0; m < k; ++m)
                    if (medoids[m] == cand) { isMed = true; break; }
                if (isMed) continue;

                QVector<int> trial = medoids;
                trial[i] = cand;
                double c = totalCost(trial, pointIdx);
                if (c < bestSwapCost) {
                    bestSwapCost = c;
                    bestSwap = cand;
                }
            }

            if (bestSwap >= 0) {
                medoids[i] = bestSwap;
                bestCost = bestSwapCost;
                swapped = true;
            }
        }
    }

    result.medoidIndices = medoids;
    result.labels = assignLabels(medoids, pointIdx);
    result.totalCost = bestCost;
    result.iterations = iter;
    result.converged = !swapped;
    return result;
}

/* ---- Full PAM fit ---- */

KMedoids22::MedoidResult KMedoids22::fitPAM(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    MedoidResult best;
    if (n < m_k) return best;

    buildDistMatrix(data);
    QVector<int> allIdx(n);
    for (int i = 0; i < n; ++i) allIdx[i] = i;

    // Multi-start PAM
    double bestCost = 1e30;
    for (int s = 0; s < m_multiStarts; ++s) {
        auto res = runPAM(allIdx, m_k);
        emit pamStartDone(s, res.totalCost);
        if (res.totalCost < bestCost) {
            bestCost = res.totalCost;
            best = res;
        }
    }

    // Remap labels from subset indices to original
    best.labels = assign(data, best.medoidIndices);

    double elapsed = timer.elapsed();
    m_stats.numPoints = n;
    m_stats.numMedoids = m_k;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit fittingDone(m_k, best.totalCost, elapsed);

    return best;
}

/* ---- CLARA fit (large datasets) ---- */

KMedoids22::MedoidResult KMedoids22::fitCLARA(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    MedoidResult best;
    if (n < m_k) return best;

    buildDistMatrix(data);
    int sampleSize = qBound(m_k + 1, static_cast<int>(m_sampleFrac * n), n);
    double bestCost = 1e30;

    for (int round = 0; round < m_claraSamples; ++round) {
        // Random sample without replacement
        QVector<int> pool(n);
        for (int i = 0; i < n; ++i) pool[i] = i;
        for (int i = n - 1; i > 0; --i) {
            int j = static_cast<int>(QRandomGenerator::global()->bounded(i + 1));
            std::swap(pool[i], pool[j]);
        }
        QVector<int> sample(sampleSize);
        for (int i = 0; i < sampleSize; ++i) sample[i] = pool[i];

        // Run PAM on sample with multi-starts
        for (int s = 0; s < m_multiStarts; ++s) {
            auto res = runPAM(sample, m_k);
            // Evaluate cost on full dataset
            QVector<int> fullIdx(n);
            for (int i = 0; i < n; ++i) fullIdx[i] = i;
            double fullCost = totalCost(res.medoidIndices, fullIdx);
            if (fullCost < bestCost) {
                bestCost = fullCost;
                best = res;
                best.totalCost = fullCost;
            }
        }
        emit claraRoundDone(round, bestCost);
    }

    best.labels = assign(data, best.medoidIndices);

    double elapsed = timer.elapsed();
    m_stats.numPoints = n;
    m_stats.numMedoids = m_k;
    m_stats.claraSamples = m_claraSamples;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit fittingDone(m_k, best.totalCost, elapsed);

    return best;
}

/* ---- Assign new points to nearest medoid ---- */

QVector<int> KMedoids22::assign(const QVector<QVector<double>>& data,
                                 const QVector<int>& medoidIdx) const
{
    int n = data.size();
    QVector<int> labels(n, 0);
    for (int i = 0; i < n; ++i) {
        double minD = 1e30;
        int best = 0;
        for (int j = 0; j < medoidIdx.size(); ++j) {
            int mi = medoidIdx[j];
            if (mi >= 0 && mi < data.size()) {
                double d = euclidean(data[i], data[mi]);
                if (d < minD) { minD = d; best = j; }
            }
        }
        labels[i] = best;
    }
    return labels;
}

/* ---- Reset ---- */

void KMedoids22::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_dist.clear();
}
