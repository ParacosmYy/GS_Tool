/**
 * @file MeanShift9.cpp
 * @brief MeanShift9 实现
 *
 * 实现均值漂移聚类：KNN可变带宽、核函数加权重心漂移、吸引盆地合并。
 */

#include "utils/cluster187/MeanShift9.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

MeanShift9::MeanShift9(QObject *parent) : QObject(parent) {}
MeanShift9::~MeanShift9() = default;

/* ---- Configuration ---- */

void MeanShift9::setBandwidth(double h) { m_bandwidth = qMax(0.01, h); }
void MeanShift9::setKnnK(int k) { m_knnK = qMax(3, k); }
void MeanShift9::setMaxIterations(int iter) { m_maxIterations = qMax(10, iter); }
void MeanShift9::setTolerance(double tol) { m_tolerance = qMax(1e-10, tol); }
void MeanShift9::setMergeThreshold(double t) { m_mergeThreshold = qMax(0.001, t); }
void MeanShift9::setKernel(Kernel k) { m_kernel = k; }

/* ---- Euclidean distance ---- */

double MeanShift9::distance(const QVector<double>& a,
                            const QVector<double>& b) const
{
    double sum = 0.0;
    for (int i = 0; i < a.size(); ++i) {
        double d = a[i] - b[i];
        sum += d * d;
    }
    return qSqrt(sum);
}

/* ---- Compute per-point bandwidth via KNN distance ---- */

void MeanShift9::computeKnnBandwidths(const QVector<QVector<double>>& data)
{
    int N = data.size();
    m_bandwidths.resize(N);

    for (int i = 0; i < N; ++i) {
        // Collect distances to all other points
        QVector<QPair<double, int>> dists;
        dists.reserve(N - 1);
        for (int j = 0; j < N; ++j) {
            if (i == j) continue;
            dists.append({distance(data[i], data[j]), j});
        }
        // Sort ascending and take k-th distance as local bandwidth
        std::sort(dists.begin(), dists.end());
        int k = qMin(m_knnK, dists.size()) - 1;
        m_bandwidths[i] = qMax(m_bandwidth,
                               (k >= 0 ? dists[k].first : m_bandwidth));
    }
}

/* ---- Kernel weight ---- */

double MeanShift9::kernelWeight(double dist, double h) const
{
    double u = dist / h;
    if (u > 1.0 && m_kernel != Gaussian) return 0.0;

    switch (m_kernel) {
    case Gaussian:
        return qExp(-0.5 * u * u);
    case Epanechnikov:
        return 0.75 * (1.0 - u * u);
    case Flat:
        return 1.0;
    }
    return 1.0;
}

/* ---- Single point mean shift ---- */

QVector<double> MeanShift9::shiftPoint(
    const QVector<double>& point,
    const QVector<QVector<double>>& data,
    double h) const
{
    int D = point.size();
    QVector<double> numerator(D, 0.0);
    double denominator = 0.0;

    for (int i = 0; i < data.size(); ++i) {
        double d = distance(point, data[i]);
        double w = kernelWeight(d, h);
        if (w < 1e-12) continue;
        for (int dim = 0; dim < D; ++dim)
            numerator[dim] += w * data[i][dim];
        denominator += w;
    }

    if (denominator < 1e-12) return point;

    QVector<double> shifted(D);
    for (int dim = 0; dim < D; ++dim)
        shifted[dim] = numerator[dim] / denominator;
    return shifted;
}

/* ---- Merge nearby modes into cluster labels ---- */

QVector<int> MeanShift9::mergeModes(double threshold)
{
    int n = m_modes.size();
    QVector<int> labels(n, -1);
    int clusterId = 0;

    for (int i = 0; i < n; ++i) {
        if (labels[i] >= 0) continue;
        labels[i] = clusterId;
        for (int j = i + 1; j < n; ++j) {
            if (labels[j] >= 0) continue;
            if (distance(m_modes[i], m_modes[j]) < threshold)
                labels[j] = clusterId;
        }
        clusterId++;
    }
    return labels;
}

/* ---- Main fit ---- */

QVector<int> MeanShift9::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    int N = data.size();
    if (N == 0) return {};

    // Step 1: Compute variable bandwidths via KNN
    computeKnnBandwidths(data);

    // Step 2: Shift each point until convergence
    m_modes.resize(N);
    for (int i = 0; i < N; ++i)
        m_modes[i] = data[i];

    int totalIter = 0;
    for (int i = 0; i < N; ++i) {
        for (int iter = 0; iter < m_maxIterations; ++iter) {
            double h = m_bandwidths[i];
            auto shifted = shiftPoint(m_modes[i], data, h);
            double move = distance(m_modes[i], shifted);
            m_modes[i] = shifted;
            if (move < m_tolerance) { totalIter += iter + 1; break; }
            if (iter == m_maxIterations - 1) totalIter += m_maxIterations;
        }
    }

    // Step 3: Merge modes into clusters
    auto modeLabels = mergeModes(m_mergeThreshold);

    // Relabel to 0..K-1
    int maxLabel = *std::max_element(modeLabels.begin(), modeLabels.end());
    QVector<int> pointLabels(N);
    for (int i = 0; i < N; ++i)
        pointLabels[i] = modeLabels[i];

    m_stats.totalRuns++;
    m_stats.numPoints = N;
    m_stats.numClusters = maxLabel + 1;
    m_stats.iterationsUsed = totalIter;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalRuns;

    emit clusteringCompleted(maxLabel + 1, totalIter, timer.elapsed());
    return pointLabels;
}

/* ---- Reset ---- */

void MeanShift9::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_modes.clear();
    m_bandwidths.clear();
}
