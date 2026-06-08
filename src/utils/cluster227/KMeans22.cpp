/**
 * @file KMeans22.cpp
 * @brief KMeans22 实现
 *
 * 实现K均值聚类：小批量随机梯度更新与余弦退火自适应学习率。
 */

#include "utils/cluster227/KMeans22.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

KMeans22::KMeans22(QObject *parent) : QObject(parent) {}
KMeans22::~KMeans22() = default;

/* ---- LCG random generator ---- */

double KMeans22::randUniform()
{
    m_seed = (m_seed * 1103515245 + 12345) & 0x7FFFFFFF;
    return static_cast<double>(m_seed) / 0x7FFFFFFF;
}

/* ---- Configuration ---- */

void KMeans22::setParameters(int numClusters, int batchSize, int maxIter)
{
    m_numClusters = qMax(2, numClusters);
    m_batchSize = qMax(8, batchSize);
    m_maxIter = qMax(10, maxIter);
}

/* ---- Squared Euclidean distance ---- */

double KMeans22::squaredDist(const QVector<double>& a,
                              const QVector<double>& b) const
{
    double sum = 0.0;
    int d = qMin(a.size(), b.size());
    for (int i = 0; i < d; ++i) {
        double diff = a[i] - b[i];
        sum += diff * diff;
    }
    return sum;
}

/* ---- Nearest centroid ---- */

int KMeans22::nearestCentroid(const QVector<double>& point) const
{
    double bestDist = std::numeric_limits<double>::max();
    int bestIdx = 0;
    for (int c = 0; c < m_centroids.size(); ++c) {
        double d = squaredDist(point, m_centroids[c].coords);
        if (d < bestDist) { bestDist = d; bestIdx = c; }
    }
    return bestIdx;
}

/* ---- Cosine annealing learning rate ---- */

double KMeans22::cosineAnnealing(int iter) const
{
    // Cosine annealing: lr = 0.5 * (1 + cos(pi * iter / maxIter))
    double progress = static_cast<double>(iter) / qMax(1, m_maxIter);
    return 0.5 * (1.0 + qCos(M_PI * progress));
}

/* ---- K-means++ initialization ---- */

void KMeans22::kmeansPPInit()
{
    int n = m_data.size();
    if (n == 0) return;

    int k = m_numClusters;
    m_centroids.resize(k);
    for (auto& c : m_centroids) c.coords.resize(m_dims, 0.0);

    // Pick first centroid randomly
    int first = static_cast<int>(randUniform() * n) % n;
    m_centroids[0].coords = m_data[first];

    QVector<double> minDist(n, std::numeric_limits<double>::max());

    for (int c = 1; c < k; ++c) {
        // Update min distances to nearest chosen centroid
        double totalDist = 0.0;
        for (int i = 0; i < n; ++i) {
            double d = squaredDist(m_data[i], m_centroids[c - 1].coords);
            if (d < minDist[i]) minDist[i] = d;
            totalDist += minDist[i];
        }

        // Weighted random selection
        double r = randUniform() * totalDist;
        double cumSum = 0.0;
        int chosen = 0;
        for (int i = 0; i < n; ++i) {
            cumSum += minDist[i];
            if (cumSum >= r) { chosen = i; break; }
        }
        m_centroids[c].coords = m_data[chosen];
    }
}

/* ---- Fit with mini-batch SGD ---- */

bool KMeans22::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    if (n < m_numClusters) return false;
    m_data = data;
    m_dims = (n > 0) ? data[0].size() : 0;

    kmeansPPInit();

    int iter = 0;
    for (iter = 0; iter < m_maxIter; ++iter) {
        double lr = cosineAnnealing(iter);

        // Mini-batch: sample a subset
        int batchSz = qMin(m_batchSize, n);
        QVector<int> batchIdx(batchSz);
        for (int b = 0; b < batchSz; ++b)
            batchIdx[b] = static_cast<int>(randUniform() * n) % n;

        // Accumulate gradient per cluster
        QVector<QVector<double>> gradSum(m_numClusters,
                                          QVector<double>(m_dims, 0.0));
        QVector<int> gradCount(m_numClusters, 0);

        for (int b = 0; b < batchSz; ++b) {
            int idx = batchIdx[b];
            int c = nearestCentroid(m_data[idx]);
            for (int d = 0; d < m_dims; ++d)
                gradSum[c][d] += m_data[idx][d];
            gradCount[c]++;
        }

        // SGD update: centroid -= lr * (centroid - mean_of_assigned)
        for (int c = 0; c < m_numClusters; ++c) {
            if (gradCount[c] == 0) continue;
            for (int d = 0; d < m_dims; ++d) {
                double meanX = gradSum[c][d] / gradCount[c];
                m_centroids[c].coords[d] += lr * (meanX - m_centroids[c].coords[d]);
            }
            m_centroids[c].count = gradCount[c];
        }

        emit iterationProgress(iter, lr);
    }

    // Compute inertia
    double iner = 0.0;
    for (int i = 0; i < n; ++i)
        iner += squaredDist(m_data[i], m_centroids[nearestCentroid(m_data[i])].coords);

    m_stats.numClusters = m_numClusters;
    m_stats.numDimensions = m_dims;
    m_stats.numPoints = n;
    m_stats.iterations = iter;
    m_stats.inertia = iner;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit fitCompleted(iter, iner, timer.elapsed());
    return true;
}

/* ---- Predict ---- */

int KMeans22::predict(const QVector<double>& point) const
{
    return nearestCentroid(point);
}

/* ---- Centroids ---- */

QVector<KMeans22::Centroid> KMeans22::centroids() const
{
    return m_centroids;
}

/* ---- Inertia ---- */

double KMeans22::inertia() const
{
    return m_stats.inertia;
}

/* ---- Reset ---- */

void KMeans22::resetStatistics()
{
    m_centroids.clear();
    m_data.clear();
    m_stats = Stats{};
    m_timeSum = 0.0;
}
